/*
* Tencent is pleased to support the open source community by making Libco available.

* Copyright (C) 2014 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/

#include "co_epoll.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/* 
 * 函数epoll_wait()的封装
 * int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
 */
int	co_epoll_wait( int epfd,struct co_epoll_res *events,int maxevents,int timeout )
{
	return epoll_wait( epfd,events->events,maxevents,timeout );
}

/*
 * 函数epoll_ctl()的封装
 * int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
 *
 * typedef union epoll_data {
 *   void *ptr;
 *   int fd;
 *   uint32_t u32;
 *   uint64_t u64;
 * } epoll_data_t;
 * 
 * struct epoll_event {
 *   uint32_t events;
 *   epoll_data_t data;
 * };
 */
int	co_epoll_ctl( int epfd,int op,int fd,struct epoll_event * ev )
{
	return epoll_ctl( epfd,op,fd,ev );
}

/* 
 * 函数epoll_create()的封装
 * int epoll_create(int size);
 */ 
int	co_epoll_create( int size )
{
	return epoll_create( size );
}

/* 分配一个struct epoll_event类型的数组 */
struct co_epoll_res *co_epoll_res_alloc( int n )
{
	struct co_epoll_res * ptr = 
		(struct co_epoll_res *)malloc( sizeof( struct co_epoll_res ) );

	ptr->size = n;
	ptr->events = (struct epoll_event*)calloc( 1,n * sizeof( struct epoll_event ) );

	return ptr;

}

/* 销毁一个struct epoll_event类型的数组 */
void co_epoll_res_free( struct co_epoll_res * ptr )
{
	if( !ptr ) return;
	if( ptr->events ) free( ptr->events );
	free( ptr );
}


