#pragma once

struct IQuoteProvider {
    virtual ~IQuoteProvider() = default;
    virtual void start() = 0;
    virtual void subscribe() = 0;
};