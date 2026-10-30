-- MySQL 8.0.16 及以上；请先创建数据库并 USE 对应数据库，再执行本文件。
-- 全部业务表使用 InnoDB，金额以 DECIMAL 存储。
CREATE TABLE IF NOT EXISTS `User` (
 id INT PRIMARY KEY AUTO_INCREMENT,
 student_id VARCHAR(32) NOT NULL UNIQUE,
 name VARCHAR(40) NOT NULL,
 password VARCHAR(128) NOT NULL,
 contact VARCHAR(120) NOT NULL,
 role TINYINT NOT NULL DEFAULT 1 CHECK (role IN (1,2)),
 status TINYINT NOT NULL DEFAULT 0 CHECK (status IN (0,1,3)),
 credit_score DECIMAL(5,2) NOT NULL DEFAULT 100 CHECK (credit_score BETWEEN 0 AND 100),
 created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS Category (
 id INT PRIMARY KEY AUTO_INCREMENT,
 name VARCHAR(40) NOT NULL UNIQUE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS Goods (
 id INT PRIMARY KEY AUTO_INCREMENT,
 name VARCHAR(160) NOT NULL,
 price DECIMAL(10,2) NOT NULL CHECK (price >= 0),
 image_path VARCHAR(500) NOT NULL DEFAULT '',
 description TEXT NOT NULL,
 condition_desc VARCHAR(200) NOT NULL,
 category_id INT NOT NULL,
 publisher_id INT NOT NULL,
 status ENUM('available','reserved','sold') NOT NULL DEFAULT 'available',
 is_deleted TINYINT NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),
 created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
 FOREIGN KEY (category_id) REFERENCES Category(id),
 FOREIGN KEY (publisher_id) REFERENCES `User`(id),
 INDEX idx_goods_list(is_deleted,status,category_id),
 FULLTEXT INDEX ft_goods(name,description) WITH PARSER ngram
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS PurchaseRequest (
 id INT PRIMARY KEY AUTO_INCREMENT,
 goods_id INT NOT NULL,
 buyer_id INT NOT NULL,
 status ENUM('pending','accepted','rejected','withdrawn') NOT NULL DEFAULT 'pending',
 created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
 pending_key INT GENERATED ALWAYS AS (CASE WHEN status='pending' THEN goods_id ELSE NULL END) STORED,
 UNIQUE KEY uq_pending_request(buyer_id,pending_key),
 FOREIGN KEY (goods_id) REFERENCES Goods(id),
 FOREIGN KEY (buyer_id) REFERENCES `User`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS TradeOrder (
 id INT PRIMARY KEY AUTO_INCREMENT,
 request_id INT NOT NULL UNIQUE,
 goods_id INT NOT NULL,
 buyer_id INT NOT NULL,
 seller_id INT NOT NULL,
 amount DECIMAL(10,2) NOT NULL CHECK (amount >= 0),
 status ENUM('confirmed','completed','cancelled') NOT NULL DEFAULT 'confirmed',
 created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
 completed_at DATETIME NULL,
 -- 唯一索引允许多条 NULL，保留多次取消尝试，但最多一条有效或成交订单。
 live_goods_id INT GENERATED ALWAYS AS (CASE WHEN status IN ('confirmed','completed') THEN goods_id ELSE NULL END) STORED,
 UNIQUE KEY uq_one_live_order(live_goods_id),
 CHECK (buyer_id <> seller_id),
 FOREIGN KEY (request_id) REFERENCES PurchaseRequest(id),
 FOREIGN KEY (goods_id) REFERENCES Goods(id),
 FOREIGN KEY (buyer_id) REFERENCES `User`(id),
 FOREIGN KEY (seller_id) REFERENCES `User`(id),
 INDEX idx_order_seller(seller_id,status),
 INDEX idx_order_buyer(buyer_id,status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 每单一条双向评价：买家评价卖家、卖家评价买家各占一组字段。
CREATE TABLE IF NOT EXISTS Review (
 id INT PRIMARY KEY AUTO_INCREMENT,
 order_id INT NOT NULL UNIQUE,
 buyer_rating TINYINT NULL CHECK (buyer_rating BETWEEN 1 AND 5),
 buyer_comment VARCHAR(500) NULL,
 buyer_reviewed_at DATETIME NULL,
 seller_rating TINYINT NULL CHECK (seller_rating BETWEEN 1 AND 5),
 seller_comment VARCHAR(500) NULL,
 seller_reviewed_at DATETIME NULL,
 FOREIGN KEY (order_id) REFERENCES TradeOrder(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS Report (
 id INT PRIMARY KEY AUTO_INCREMENT,
 reporter_id INT NOT NULL,
 target_type ENUM('goods','user') NOT NULL,
 goods_id INT NULL,
 user_id INT NULL,
 reason VARCHAR(500) NOT NULL,
 status ENUM('pending','resolved','dismissed') NOT NULL DEFAULT 'pending',
 handler_id INT NULL,
 result_note VARCHAR(500) NULL,
 created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
 handled_at DATETIME NULL,
 -- 两个可空外键配合 CHECK，杜绝不存在的多态对象 ID。
 CHECK ((target_type='goods' AND goods_id IS NOT NULL AND user_id IS NULL)
     OR (target_type='user' AND user_id IS NOT NULL AND goods_id IS NULL)),
 UNIQUE KEY uq_goods_report(reporter_id,goods_id),
 UNIQUE KEY uq_user_report(reporter_id,user_id),
 FOREIGN KEY (reporter_id) REFERENCES `User`(id),
 FOREIGN KEY (goods_id) REFERENCES Goods(id),
 FOREIGN KEY (user_id) REFERENCES `User`(id),
 FOREIGN KEY (handler_id) REFERENCES `User`(id),
 INDEX idx_report_status(status,target_type)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS Message (
 id INT PRIMARY KEY AUTO_INCREMENT,
 goods_id INT NOT NULL,
 sender_id INT NOT NULL,
 content VARCHAR(1000) NOT NULL,
 timestamp DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
 FOREIGN KEY (goods_id) REFERENCES Goods(id),
 FOREIGN KEY (sender_id) REFERENCES `User`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO Category(name) VALUES ('数码产品'),('学习用品'),('生活百货'),('代步工具'),('运动健身'),('其他');

DELIMITER $$
CREATE TRIGGER review_before_insert BEFORE INSERT ON Review FOR EACH ROW
BEGIN
 IF NOT EXISTS(SELECT 1 FROM TradeOrder WHERE id=NEW.order_id AND status='completed')
 OR NEW.buyer_rating IS NOT NULL OR NEW.seller_rating IS NOT NULL THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='评价记录只能在完成订单时初始化，评分须分方提交';
 END IF;
END$$

CREATE TRIGGER review_before_update BEFORE UPDATE ON Review FOR EACH ROW
BEGIN
 IF (OLD.buyer_rating IS NOT NULL AND NOT (NEW.buyer_rating <=> OLD.buyer_rating AND NEW.buyer_comment <=> OLD.buyer_comment AND NEW.buyer_reviewed_at <=> OLD.buyer_reviewed_at))
 OR (OLD.seller_rating IS NOT NULL AND NOT (NEW.seller_rating <=> OLD.seller_rating AND NEW.seller_comment <=> OLD.seller_comment AND NEW.seller_reviewed_at <=> OLD.seller_reviewed_at))
 OR NEW.order_id <> OLD.order_id THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='每方评价仅能提交一次';
 END IF;
END$$

CREATE TRIGGER order_before_update BEFORE UPDATE ON TradeOrder FOR EACH ROW
BEGIN
 IF NEW.goods_id<>OLD.goods_id OR NEW.buyer_id<>OLD.buyer_id OR NEW.seller_id<>OLD.seller_id
 OR NEW.request_id<>OLD.request_id OR NEW.amount<>OLD.amount
 OR (OLD.status<>'confirmed' AND NEW.status<>OLD.status) THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='订单主体不可修改，完成或取消后不可回退';
 END IF;
END$$

CREATE TRIGGER review_after_update AFTER UPDATE ON Review FOR EACH ROW
BEGIN
 -- 信用分 = 收到的所有评分平均值 × 20；无评价默认为 100。
 IF OLD.buyer_rating IS NULL AND NEW.buyer_rating IS NOT NULL THEN
  UPDATE `User` SET credit_score=(
   SELECT ROUND(AVG(r.rating)*20,2) FROM (
    SELECT o.seller_id uid,v.buyer_rating rating FROM Review v JOIN TradeOrder o ON o.id=v.order_id WHERE v.buyer_rating IS NOT NULL
    UNION ALL
    SELECT o.buyer_id uid,v.seller_rating rating FROM Review v JOIN TradeOrder o ON o.id=v.order_id WHERE v.seller_rating IS NOT NULL
   ) r WHERE r.uid=`User`.id
  ) WHERE id=(SELECT seller_id FROM TradeOrder WHERE id=NEW.order_id);
 END IF;
 IF OLD.seller_rating IS NULL AND NEW.seller_rating IS NOT NULL THEN
  UPDATE `User` SET credit_score=(
   SELECT ROUND(AVG(r.rating)*20,2) FROM (
    SELECT o.seller_id uid,v.buyer_rating rating FROM Review v JOIN TradeOrder o ON o.id=v.order_id WHERE v.buyer_rating IS NOT NULL
    UNION ALL
    SELECT o.buyer_id uid,v.seller_rating rating FROM Review v JOIN TradeOrder o ON o.id=v.order_id WHERE v.seller_rating IS NOT NULL
   ) r WHERE r.uid=`User`.id
  ) WHERE id=(SELECT buyer_id FROM TradeOrder WHERE id=NEW.order_id);
 END IF;
END$$
DELIMITER ;
