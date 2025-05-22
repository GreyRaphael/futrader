#pragma once

struct IQuoteProvider {
    virtual ~IQuoteProvider() = default;
    virtual void Start() = 0;
    virtual void Subscribe() = 0;
};