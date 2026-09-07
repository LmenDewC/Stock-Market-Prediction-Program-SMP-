#pragma once

#include <string>
#include <vector>

struct Bar {
    std::string symbol;
    std::string timestamp;

    double open{};
    double high{};
    double low{};
    double close{};
    double volume{};
    double vwap{};

    long long trade_count{};
};

class AlpacaClient {
public:
    AlpacaClient(
        std::string api_key,
        std::string api_secret
    );

    std::vector<Bar> get_bars(
        const std::vector<std::string>& symbols,
        const std::string& timeframe,
        const std::string& start,
        const std::string& end
    );

private:
    std::string api_key_;
    std::string api_secret_;

    std::string request(
        const std::string& url
    ) const;
};
