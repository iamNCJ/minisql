# Interpreter API

解析命令 + 调用核心功能



# Catalog 

维护数据库、表的定义



# Index 

维护B树的索引



# Record

操作B树（由Buffer提供接口）



# Buffer 

维护B树的block（磁盘中）+ 使用缓冲区进行优化加速，最后统一包装成接口 （可以先写B树确定接口，再去实现缓冲）





