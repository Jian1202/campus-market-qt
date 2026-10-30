"""创建全新实验数据库；不覆盖已有业务表。需要 PyMySQL。"""
import argparse
import configparser
import getpass
import hashlib
import os
from pathlib import Path
import re
import secrets
import pymysql

ROOT = Path(__file__).resolve().parents[1]

def statements(text):
    """识别 mysql 客户端 DELIMITER，包括触发器内部的分号。"""
    delimiter = ';'
    buffer = ''
    for line in text.splitlines():
        if line.strip().startswith('--') or not line.strip():
            continue
        if line.strip().upper().startswith('DELIMITER '):
            delimiter = line.strip().split(None, 1)[1]
            continue
        buffer += line + '\n'
        if buffer.rstrip().endswith(delimiter):
            yield buffer.rstrip()[:-len(delimiter)]
            buffer = ''
    if buffer.strip():
        raise ValueError('SQL 文件末尾缺少语句分隔符')

def password_hash(password):
    salt = secrets.token_bytes(16)
    return salt.hex() + '$' + hashlib.pbkdf2_hmac('sha256', password.encode(), salt, 120000).hex()

def initialize(connection, database, with_history=True):
    if not re.fullmatch(r'[a-zA-Z0-9_]+', database):
        raise ValueError('数据库名仅能包含字母、数字和下划线')
    with connection.cursor() as cur:
        cur.execute('CREATE DATABASE IF NOT EXISTS `' + database + '` CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci')
        cur.execute('USE `' + database + '`')
        cur.execute('SHOW TABLES')
        if cur.fetchall():
            raise RuntimeError('目标数据库已有表，已停止；请使用新的数据库名，避免覆盖数据')
        for sql in statements((ROOT / 'database/schema.sql').read_text(encoding='utf-8')):
            cur.execute(sql)
        files = ['seed.sql'] + (['demo.sql', 'demo_products.sql'] if with_history else [])
        for filename in files:
            for sql in statements((ROOT / 'database' / filename).read_text(encoding='utf-8')):
                cur.execute(sql)
    connection.commit()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('--port', type=int, default=3306)
    parser.add_argument('--user', default='root')
    parser.add_argument('--database', default='campus_market')
    parser.add_argument('--config-dir', type=Path, default=ROOT / '运行程序')
    parser.add_argument('--empty-admin-password', action='store_true', help='仅用于隔离测试实例')
    parser.add_argument('--basic-test-data', action='store_true', help='不导入报表演示历史，仅用于自动测试')
    args = parser.parse_args()
    password = '' if args.empty_admin_password else os.environ.get('CAMPUS_ADMIN_PASSWORD')
    if password is None:
        password = getpass.getpass('MySQL 管理员密码（输入不显示）：')
    connection = pymysql.connect(host=args.host, port=args.port, user=args.user, password=password, charset='utf8mb4')
    try:
        initialize(connection, args.database, not args.basic_test_data)
        app_password = secrets.token_urlsafe(24)
        # 业务账号限定到本机，只有查询、插入和修改权限，不能物理删除业务记录。
        app_user = 'campus_' + secrets.token_hex(4)
        with connection.cursor() as cur:
            cur.execute('CREATE USER %s@%s IDENTIFIED BY %s', (app_user, 'localhost', app_password))
            cur.execute('GRANT SELECT,INSERT,UPDATE ON `' + args.database + '`.* TO %s@%s', (app_user, 'localhost'))
        args.config_dir.mkdir(parents=True, exist_ok=True)
        cfg = configparser.ConfigParser()
        cfg['mysql'] = {'host': args.host, 'port': str(args.port), 'database': args.database, 'user': app_user, 'password': app_password}
        target = args.config_dir / 'database.ini'
        if target.exists():
            archive = target.parent / '存档' / __import__('datetime').datetime.now().strftime('%Y%m%d-%H%M%S-%f')
            archive.mkdir(parents=True)
            target.rename(archive / target.name)
        with target.open('w', encoding='utf-8') as stream:
            cfg.write(stream)
        print('数据库初始化完成，连接配置已写入：', target)
        print('演示管理员：admin / admin123；普通账号：202600000001—202600000004 / 123456')
    finally:
        connection.close()

if __name__ == '__main__':
    main()
