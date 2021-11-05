﻿/**
 **	\file Ajp13Socket.h
 **	\date  2007-10-05
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2007-2011 Anders Hedstrom

This library is made available under the terms of the GNU GPL, with
the additional exemption that compiling, linking, and/or using OpenSSL 
is allowed.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef _SOCKETS_Ajp13Socket_H
#define _SOCKETS_Ajp13Socket_H

#include "AjpBaseSocket.h"
#include "HttpRequest.h"
#include "IHttpServer.h"
#include "HttpResponse.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


class HttpResponse;

class Ajp13Socket : public AjpBaseSocket, public IHttpServer
{
public:
	Ajp13Socket(ISocketHandler& h);

	void OnHeader( short id, short len );
	void OnPacket( const char *buf, size_t sz );

	// implements IHttpServer::Respond
	void IHttpServer_Respond(const HttpResponse& res);

	void OnTransferLimit();

	void SetReused(bool x = true);
	bool IsReused();

private:
	void ReceiveBody( const char *buf, size_t sz );
	void ReceiveForwardRequest( const char *buf, size_t sz );
	void ReceiveShutdown( const char *buf, size_t sz );
	void ReceivePing( const char *buf, size_t sz );
	void ReceiveCPing( const char *buf, size_t sz );
	void Execute();
	void Reset();
	//
	size_t m_body_size_left;
	HttpRequest m_req;
	HttpResponse m_res;
	bool m_b_reused;
};


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif

#endif // _SOCKETS_Ajp13Socket_H

