-- 额外演示数据：两个分类的成交、低信用活跃卖家和超过阈值的举报。
INSERT INTO Goods(name,price,description,condition_desc,category_id,publisher_id,image_path,status) VALUES
('已成交的参考书',30,'演示历史成交','七成新',2,4,':/resources/images/preset/复习资料.png','sold'),
('已成交的旧耳机',60,'演示历史成交','八成新',1,4,':/resources/images/preset/耳机.png','sold');
INSERT INTO PurchaseRequest(goods_id,buyer_id,status) VALUES(6,3,'accepted'),(7,5,'accepted');
INSERT INTO TradeOrder(request_id,goods_id,buyer_id,seller_id,amount,status,completed_at) VALUES
(1,6,3,4,30,'completed',NOW()),(2,7,5,4,60,'completed',NOW());
INSERT INTO Review(order_id) VALUES(1),(2);
UPDATE Review SET buyer_rating=2,buyer_comment='演示：商品成色不符',buyer_reviewed_at=NOW(),seller_rating=4,seller_comment='演示：面交及时',seller_reviewed_at=NOW() WHERE order_id=1;
UPDATE Review SET buyer_rating=3,buyer_comment='演示：有磨损',buyer_reviewed_at=NOW(),seller_rating=5,seller_comment='演示：沟通顺畅',seller_reviewed_at=NOW() WHERE order_id=2;
INSERT INTO Goods(name,price,description,condition_desc,category_id,publisher_id,image_path) VALUES('低信用卖家在售教材',20,'演示活跃卖家的当前商品','七成新',2,4,':/resources/images/preset/复习资料.png');
INSERT INTO Report(reporter_id,target_type,goods_id,reason) VALUES(3,'goods',2,'演示举报：描述有误'),(4,'goods',2,'演示举报：涉嫌重复发布'),(5,'goods',2,'演示举报：价格描述不符');
INSERT INTO Report(reporter_id,target_type,user_id,reason) VALUES(3,'user',2,'演示举报：发布不实信息'),(4,'user',2,'演示举报：疑似违规'),(5,'user',2,'演示举报：沟通不当');
