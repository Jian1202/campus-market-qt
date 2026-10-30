-- 报表一：分类成交数量和成交均价；软删除不抹去历史成交。
SELECT c.name AS 分类, COUNT(o.id) AS 成交数, ROUND(AVG(o.amount),2) AS 成交均价
FROM Category c JOIN Goods g ON g.category_id=c.id
JOIN TradeOrder o ON o.goods_id=g.id
WHERE o.status='completed'
GROUP BY c.id,c.name ORDER BY 成交数 DESC;

-- 报表二：信用分低于 80 且近 30 天发布过商品的活跃卖家。
SELECT u.id AS 用户编号,u.name AS 卖家,u.credit_score AS 信用分,
 COUNT(DISTINCT g.id) AS 近30天发布数,COUNT(DISTINCT o.id) AS 成交数
FROM `User` u JOIN Goods g ON g.publisher_id=u.id
LEFT JOIN TradeOrder o ON o.goods_id=g.id AND o.status='completed'
WHERE u.role=1 AND u.status=0 AND u.credit_score<80
AND g.created_at>=DATE_SUB(NOW(),INTERVAL 30 DAY)
GROUP BY u.id,u.name,u.credit_score ORDER BY u.credit_score;

-- 报表三：订单交易明细，连接订单、商品、分类、买家、卖家和评价。
SELECT o.id AS 订单号,g.name AS 商品,c.name AS 分类,b.name AS 买家,
 s.name AS 卖家,o.amount AS 成交金额,o.status AS 订单状态,
 r.buyer_rating AS 买家评分,r.seller_rating AS 卖家评分
FROM TradeOrder o JOIN Goods g ON g.id=o.goods_id
JOIN Category c ON c.id=g.category_id JOIN `User` b ON b.id=o.buyer_id
JOIN `User` s ON s.id=o.seller_id LEFT JOIN Review r ON r.order_id=o.id
ORDER BY o.id DESC;

-- 报表四：待处理举报严格超过 2 次的商品，COUNT DISTINCT 防止重复计数。
SELECT g.id AS 商品编号,g.name AS 商品,u.name AS 卖家,COUNT(DISTINCT r.reporter_id) AS 举报人数
FROM Report r JOIN Goods g ON g.id=r.goods_id JOIN `User` u ON u.id=g.publisher_id
WHERE r.status='pending' GROUP BY g.id,g.name,u.name
HAVING COUNT(DISTINCT r.reporter_id)>2;

-- 报表五：待处理举报严格超过 2 次的用户。
SELECT u.id AS 用户编号,u.name AS 用户,u.credit_score AS 信用分,COUNT(DISTINCT r.reporter_id) AS 举报人数
FROM Report r JOIN `User` u ON u.id=r.user_id
WHERE r.status='pending' GROUP BY u.id,u.name,u.credit_score
HAVING COUNT(DISTINCT r.reporter_id)>2;
