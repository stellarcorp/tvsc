// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SoapyServer.hpp"
#include "SoapyRemoteDefs.hpp"
#include "ClientHandler.hpp"
#include "SoapyRPCSocket.hpp"
#include <thread>

#include "glog/logging.h"

/***********************************************************************
 * Server thread implementation
 **********************************************************************/
SoapyServerThreadData::SoapyServerThreadData(void):
    done(false),
    thread(nullptr),
    client(nullptr)
{
    return;
}

SoapyServerThreadData::~SoapyServerThreadData(void)
{
    done = true;
    if (thread != nullptr)
    {
        thread->join();
    }
    delete thread;
    if (client != nullptr)
    {
        LOG(INFO) << "SoapyServerListener::close()";
    }
    delete client;
}

void SoapyServerThreadData::handlerLoop(std::function<bool(const SoapySDR::Kwargs& device)> deviceFilter)
{
    SoapyClientHandler handler(*client, uuid, std::move(deviceFilter));

    try
    {
        while (handler.handleOnce())
        {
            if (done) break;
        }
	LOG(INFO) << "Exiting handler.";
    }
    catch (const std::exception &ex)
    {
        LOG(ERROR) << "SoapyServerListener::handlerLoop() FAIL: " << ex.what();
    }

    done = true;
}

/***********************************************************************
 * Socket listener constructor
 **********************************************************************/
SoapyServerListener::SoapyServerListener(SoapyRPCSocket &sock, const std::string &uuid,
					 std::function<bool(const SoapySDR::Kwargs& device)> deviceFilter):
    _sock(sock),
    _uuid(uuid),
    _deviceFilter(std::move(deviceFilter)),
    _handlerId(0)
{
    return;
}

SoapyServerListener::~SoapyServerListener(void)
{
    auto it = _handlers.begin();
    while (it != _handlers.end())
    {
        _handlers.erase(it++);
    }
}

/***********************************************************************
 * Client socket acceptor
 **********************************************************************/
void SoapyServerListener::handleOnce(void)
{
    //cleanup completed threads
    auto it = _handlers.begin();
    while (it != _handlers.end())
    {
        auto &data = it->second;
        if (not data.done) ++it;
        else _handlers.erase(it++);
    }

    //wait with timeout for the server socket to become ready to accept
    if (not _sock.selectRecv(SOAPY_REMOTE_SOCKET_TIMEOUT_US)) return;

    SoapyRPCSocket *client = _sock.accept();
    if (client == NULL)
    {
        LOG(ERROR) << "SoapyServerListener::accept() FAIL:" << _sock.lastErrorMsg();
        return;
    }
    LOG(INFO) << "SoapyServerListener::accept(" << client->getpeername() << ")";

    //setup the thread data
    auto &data = _handlers[_handlerId++];
    data.client = client;
    data.uuid = _uuid;

    //spawn a new thread
    // Note that we copy the deviceFilter into the this thread, rather than moving it. We do this so
    // that the deviceFilter in the SoapyServerListener instance remains valid and can be reused to
    // handle subsequent requests.
    data.thread = new std::thread(&SoapyServerThreadData::handlerLoop, &data, _deviceFilter);
}
