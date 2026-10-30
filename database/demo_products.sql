-- 补充商品为虚构教学样例，网络配图来源见 docs/image-sources.md。
-- 可重复执行；不会重新上架已被下架的样例商品。
INSERT INTO Goods(name,price,description,condition_desc,category_id,publisher_id,image_path)
SELECT '课外阅读书籍一套',28,'课余阅读的几本书一起转让，内容完整，适合宿舍或自习室阅读。','八成新，书角轻微磨损',c.id,u.id,'assets/products/books.jpg' FROM `User` u JOIN Category c ON c.name='学习用品' WHERE u.student_id='202600000002' AND NOT EXISTS (SELECT 1 FROM Goods WHERE name='课外阅读书籍一套' AND image_path='assets/products/books.jpg');
INSERT INTO Goods(name,price,description,condition_desc,category_id,publisher_id,image_path)
SELECT '桌面阅读台灯',35,'宿舍桌面用的阅读台灯，开关正常，灯罩及支架完整，仅转让台灯。','九成新，正常使用痕迹',c.id,u.id,'assets/products/lamp.jpg' FROM `User` u JOIN Category c ON c.name='生活百货' WHERE u.student_id='202600000003' AND NOT EXISTS (SELECT 1 FROM Goods WHERE name='桌面阅读台灯' AND image_path='assets/products/lamp.jpg');
INSERT INTO Goods(name,price,description,condition_desc,category_id,publisher_id,image_path)
SELECT '头戴式耳机',160,'学习听力、听歌都可以，左右声道正常，耳罩已清洁，支持校园面交试听。','八成新，耳罩完好',c.id,u.id,'assets/products/headphones.jpg' FROM `User` u JOIN Category c ON c.name='数码产品' WHERE u.student_id='202600000001' AND NOT EXISTS (SELECT 1 FROM Goods WHERE name='头戴式耳机' AND image_path='assets/products/headphones.jpg');
INSERT INTO Goods(name,price,description,condition_desc,category_id,publisher_id,image_path)
SELECT '无线办公鼠标',25,'日常上课和写作业使用，按键与滚轮正常，体积小方便放进电脑包。','九成新，底部轻微磨损',c.id,u.id,'assets/products/mouse.jpg' FROM `User` u JOIN Category c ON c.name='数码产品' WHERE u.student_id='202600000002' AND NOT EXISTS (SELECT 1 FROM Goods WHERE name='无线办公鼠标' AND image_path='assets/products/mouse.jpg');
INSERT INTO Goods(name,price,description,condition_desc,category_id,publisher_id,image_path)
SELECT '黑色通勤双肩包',45,'闲置通勤背包，适合装书本和日常用品，拉链顺畅，内衬干净。','八成新，无破洞',c.id,u.id,'assets/products/backpack.jpg' FROM `User` u JOIN Category c ON c.name='生活百货' WHERE u.student_id='202600000003' AND NOT EXISTS (SELECT 1 FROM Goods WHERE name='黑色通勤双肩包' AND image_path='assets/products/backpack.jpg');
INSERT INTO Goods(name,price,description,condition_desc,category_id,publisher_id,image_path)
SELECT '纯白陶瓷马克杯',12,'简洁白色陶瓷杯，适合宿舍日常喝水，杯口无磕碰，已清洗。','九成新，无裂纹',c.id,u.id,'assets/products/mug.jpg' FROM `User` u JOIN Category c ON c.name='生活百货' WHERE u.student_id='202600000001' AND NOT EXISTS (SELECT 1 FROM Goods WHERE name='纯白陶瓷马克杯' AND image_path='assets/products/mug.jpg');
