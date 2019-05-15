httpclient: httpclient.cpp
	g++ -g -std=c++11 -o httpclient httpclient.cpp event_loop.cpp connector.cpp sockets.cpp inner_pre.cpp tcp_client.cpp co_epoll.cpp coctx.cpp coctx_swap.S co_routine.cpp co_hook_sys_call.cpp -ldl 

connector_test: connector_test.cpp connector.cpp event_loop.cpp 
	g++ -g -std=c++11 -o connector_test connector_test.cpp connector.cpp event_loop.cpp inner_pre.cpp co_epoll.cpp coctx.cpp coctx_swap.S co_routine.cpp co_hook_sys_call.cpp -ldl

event_loop_test: event_loop_test.cpp event_loop.cpp 
	g++ -g -std=c++11 -o event_loop_test event_loop_test.cpp event_loop.cpp inner_pre.cpp co_epoll.cpp coctx.cpp coctx_swap.S co_routine.cpp co_hook_sys_call.cpp -ldl

tcp_client_test: tcp_client.cpp event_loop.cpp connector.cpp tcp_client_test.cpp 
	g++ -g -std=c++11 -o tcp_client_test tcp_client_test.cpp event_loop.cpp connector.cpp tcp_client.cpp inner_pre.cpp co_epoll.cpp coctx.cpp coctx_swap.S co_routine.cpp co_hook_sys_call.cpp -ldl

clean:
	rm -rf a.out .*.swp *.gch *.*~ httpclient *.o connector_test event_loop_test tcp_client_test httpclient httpclient++
