#include <iostream>

#include <string>
#include <boost/asio.hpp>
#include "CService.h"
#include "CSession.h"


using boost::asio::ip::tcp;
using boost::asio::detached;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;




int main(){
    try{

        //获取IOServicePool对象用于退出
        auto & pool = IOServicePool::Instance();

        boost::asio::io_context ioc;

        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        // 注册信号处理器, 当接收到SIGINT或SIGTERM信号时，停止IO上下文
        signals.async_wait([&ioc,&pool](const boost::system::error_code& error, int signal_number){
            std::cout << "Signal received: " << signal_number << std::endl;
            ioc.stop();
            pool.Stop();
        });

        // 创建服务器对象
        CService service(ioc, 12345);
        std::cout << "Server started on port 12345" << std::endl;
     
        // 启动IO上下文s
        ioc.run();

    }catch(const std::exception& e){
        std::cerr << "Error: " << e.what() << std::endl;
    }



    return 0;
}




