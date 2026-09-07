#include "alpaca.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

const char* get_env(
    const char* name
) {
    const char* value =
        std::getenv(name);

    if (!value || *value == '\0') {
        throw std::runtime_error(
            std::string(
                "Missing environment variable: "
            ) + name
        );
    }

    return value;
}

int main() {

    try {

        const std::string api_key =
            get_env("ALPACA_API_KEY");

        const std::string api_secret =
            get_env("ALPACA_API_SECRET");

        AlpacaClient client(
            api_key,
            api_secret
        );

        std::vector<std::string> symbols {
            "AAPL",
            "MSFT",
            "NVDA",
            "TSLA"
        };

        std::cout
            << "Requesting market data...\n";

        auto bars = client.get_bars(
            symbols,

            // 1-minute bars
            "1Min",

            // Start
            "2026-01-01T00:00:00Z",

            // End
            "2026-02-01T00:00:00Z"
        );

        std::ofstream file(
            "market_data.csv"
        );

        if (!file) {
            throw std::runtime_error(
                "Could not open CSV file"
            );
        }

        file
            << "symbol,"
            << "timestamp,"
            << "open,"
            << "high,"
            << "low,"
            << "close,"
            << "volume,"
            << "vwap,"
            << "trade_count\n";

        for (const auto& bar : bars) {

            file
                << bar.symbol << ","
                << bar.timestamp << ","
                << bar.open << ","
                << bar.high << ","
                << bar.low << ","
                << bar.close << ","
                << bar.volume << ","
                << bar.vwap << ","
                << bar.trade_count
                << "\n";
        }

        std::cout
            << "Collected "
            << bars.size()
            << " bars.\n";

        std::cout
            << "Saved to market_data.csv\n";
    }

    catch (const std::exception& e) {

        std::cerr
            << "ERROR: "
            << e.what()
            << '\n';

        return 1;
    }

    return 0;
}
