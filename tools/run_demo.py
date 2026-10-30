"""启动独立 MySQL 演示实例与 Qt 程序，关闭程序后停止该实例。"""
import argparse
import configparser
import json
import os
from pathlib import Path
import secrets
import socket
import subprocess
import time
import pymysql
from init_database import initialize, statements, ROOT

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mysql-dir', type=Path)
    parser.add_argument('--port', type=int, default=3317)
    parser.add_argument('--prepare-only', action='store_true', help='仅初始化并验证连接，不启动窗口')
    args = parser.parse_args()
    mysql_dir = args.mysql_dir
    if mysql_dir is None:
        candidates = sorted((Path(os.environ.get('ProgramFiles', r'C:\Program Files')) / 'MySQL').glob('MySQL Server *'))
        if not candidates:
            raise RuntimeError('未找到 MySQL Server，请先安装 MySQL 8.0.16 或以上，或通过 --mysql-dir 指定目录')
        mysql_dir = candidates[-1]
    exe = mysql_dir / 'bin/mysqld.exe'
    if not exe.exists():
        raise RuntimeError('MySQL 目录中没有 bin/mysqld.exe')
    # 采用固定英文目录，避免部分 Windows MySQL 对中文数据目录处理异常。
    state_dir = Path(os.environ.get('PUBLIC', r'C:\Users\Public')) / 'CampusMarketDemo'
    state_dir.mkdir(parents=True, exist_ok=True)
    state_path = state_dir / 'instance.json'
    data = state_dir / 'data'
    if data.exists() and not state_path.exists():
        raise RuntimeError('检测到未完成初始化的数据目录，请保留现场，改用手动配置方式连接 MySQL')
    if state_path.exists():
        saved = json.loads(state_path.read_text(encoding='utf-8'))
        args.port = saved['port']
    else:
        saved = {'port': args.port, 'password': '', 'database': 'campus_market_demo'}
    with socket.socket() as sock:
        already_running = sock.connect_ex(('127.0.0.1', args.port)) == 0
    if already_running and not state_path.exists():
        raise RuntimeError(f'端口 {args.port} 被其他服务占用，未找到本演示的实例记录')
    flags = subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0
    log = (state_dir / 'launcher.log').open('ab')
    if not data.exists():
        subprocess.run([str(exe), '--no-defaults', '--initialize-insecure', '--datadir=' + str(data), '--console'],
                       stdout=log, stderr=log, creationflags=flags, check=True)
        state_path.write_text(json.dumps(saved), encoding='utf-8')
    proc = None
    if not already_running:
        proc = subprocess.Popen([str(exe), '--no-defaults', '--datadir=' + str(data), '--port=' + str(args.port),
                             '--bind-address=127.0.0.1', '--mysqlx=0', '--log-error=' + str(state_dir / 'mysql.log'),
                             '--pid-file=' + str(state_dir / 'mysql.pid')],
                            stdout=log, stderr=log, creationflags=flags)
    connection = None
    try:
        for _ in range(120):
            if proc is not None and proc.poll() is not None:
                raise RuntimeError('MySQL 启动失败，请查看 ' + str(state_dir / 'mysql.log'))
            try:
                connection = pymysql.connect(host='127.0.0.1', port=args.port, user='root', password=saved['password'], charset='utf8mb4')
                break
            except pymysql.OperationalError:
                time.sleep(.25)
        if connection is None:
            raise RuntimeError('等待 MySQL 启动超时')
        # 仅复用凭据和数据目录均匹配的本项目实例，不能接管其他 MySQL 服务。
        with connection.cursor() as cur:
            cur.execute('SELECT @@datadir')
            actual_data = Path(cur.fetchone()[0]).resolve()
        if actual_data != data.resolve():
            raise RuntimeError('端口上的 MySQL 数据目录不属于本演示，已停止启动')
        if already_running:
            print('复用已运行的本项目演示数据库，不会关闭原有数据库进程')
        if not saved['password']:
            saved['password'] = secrets.token_urlsafe(24)
            with connection.cursor() as cur:
                cur.execute('ALTER USER CURRENT_USER() IDENTIFIED BY %s', (saved['password'],))
            state_path.write_text(json.dumps(saved), encoding='utf-8')
        if 'app_user' not in saved:
            initialize(connection, saved['database'])
            saved['app_user'] = 'campus_demo'
            saved['app_password'] = secrets.token_urlsafe(24)
            with connection.cursor() as cur:
                cur.execute('CREATE USER %s@%s IDENTIFIED BY %s', (saved['app_user'], 'localhost', saved['app_password']))
                cur.execute('GRANT SELECT,INSERT,UPDATE ON campus_market_demo.* TO %s@%s', (saved['app_user'], 'localhost'))
            state_path.write_text(json.dumps(saved), encoding='utf-8')
        # 已有演示库也增量补充样例商品，重复启动不产生重复记录。
        connection.select_db(saved['database'])
        with connection.cursor() as cur:
            for sql in statements((ROOT / 'database/demo_products.sql').read_text(encoding='utf-8')):
                cur.execute(sql)
        connection.commit()
        folder = ROOT / '运行程序'
        folder.mkdir(exist_ok=True)
        cfg = configparser.ConfigParser()
        cfg['mysql'] = {'host': '127.0.0.1', 'port': str(args.port), 'database': saved['database'],
                        'user': saved['app_user'], 'password': saved['app_password']}
        target = folder / 'database.ini'
        # 只有配置实际变化时归档，日常启动不累积重复文件。
        import io
        content = io.StringIO()
        cfg.write(content)
        if not target.exists() or target.read_text(encoding='utf-8') != content.getvalue():
            if target.exists():
                archive = folder / '存档' / time.strftime('%Y%m%d-%H%M%S')
                archive.mkdir(parents=True, exist_ok=False)
                target.rename(archive / target.name)
            target.write_text(content.getvalue(), encoding='utf-8')
        print('独立演示数据库已就绪；数据保留在', state_dir)
        print('管理员：admin / admin123；学生：202600000001—202600000004 / 123456')
        if not args.prepare_only:
            app = folder / 'CampusMarket.exe'
            if not app.exists():
                raise RuntimeError('未找到运行程序/CampusMarket.exe，请先编译并部署')
            subprocess.run([str(app)], cwd=folder, check=True)
    finally:
        stop_owned_server = proc is not None
        if connection is not None:
            if stop_owned_server:
                try:
                    # 其他窗口仍在使用时保留实例，下一次启动可安全复用。
                    with connection.cursor() as cur:
                        cur.execute('SELECT COUNT(*) FROM information_schema.PROCESSLIST WHERE USER=%s', (saved.get('app_user', ''),))
                        stop_owned_server = cur.fetchone()[0] == 0
                except pymysql.Error:
                    stop_owned_server = False
            if stop_owned_server:
                try:
                    with connection.cursor() as cur:
                        cur.execute('SHUTDOWN')
                except pymysql.Error:
                    pass
            connection.close()
        if stop_owned_server:
            try:
                proc.wait(timeout=20)
            except subprocess.TimeoutExpired:
                proc.terminate()
                proc.wait(timeout=10)
        log.close()

if __name__ == '__main__':
    main()
