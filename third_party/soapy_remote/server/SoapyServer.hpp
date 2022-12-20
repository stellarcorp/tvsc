// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <csignal> //sig_atomic_t
#include <string>
#include <thread>
#include <map>
#include <functional>

#include "SoapySDR/Device.hpp"

class SoapyRPCSocket;

//! Client handler data
struct SoapyServerThreadData
{
    SoapyServerThreadData(void);
    ~SoapyServerThreadData(void);
    void handlerLoop(std::function<bool(const SoapySDR::Kwargs& device)> deviceFilter);
    sig_atomic_t done;
    std::thread *thread;
    SoapyRPCSocket *client;
    std::string uuid;
};

/*!
 * The server listener class accepts clients and spawns threads.
 */
class SoapyServerListener
{
public:
    SoapyServerListener(SoapyRPCSocket &sock, const std::string &uuid,
			std::function<bool(const SoapySDR::Kwargs& device)> deviceFilter);

    ~SoapyServerListener(void);

    void handleOnce(void);

private:
    SoapyRPCSocket &_sock;
    const std::string _uuid;
    std::function<bool(const SoapySDR::Kwargs& device)> _deviceFilter;
    size_t _handlerId;
    std::map<size_t, SoapyServerThreadData> _handlers;
};
