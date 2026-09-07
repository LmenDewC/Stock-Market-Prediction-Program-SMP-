#include "alpaca.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <sstream>
#include <stdexcept>
#include <utility>
#include <iostream>

using json = nlohmann::json;

namespace {

size_t write_callback(
    void* contents,
    size_t size,
    size_t nmemb,
    void* userp
) {
    size_t total = size * nmemb;

    auto* buffer =
        static_cast<std::string*>(userp);

    buffer->append(
        static_cast<char*>(contents),
        total
    );

    return total;
}

}

AlpacaClient::AlpacaClient(
    std::string api_key,
    std::string api_secret
)
    : api_key_(std::move(api_key)),
      api_secret_(std::move(api_secret))
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

std::string AlpacaClient::request(
    const std::string& url
) const {
    CURL* curl = curl_easy_init();

    if (!curl) {
        throw std::runtime_error(
            "Failed to initialize libcurl"
        );
    }

    std::string response;

    struct curl_slist* headers = nullptr;

    std::string key_header =
        "APCA-API-KEY-ID: " + api_key_;

    std::string secret_header =
        "APCA-API-SECRET-KEY: " + api_secret_;

    headers = curl_slist_append(
        headers,
        key_header.c_str()
    );

    headers = curl_slist_append(
        headers,
        secret_header.c_str()
    );

    headers = curl_slist_append(
        headers,
        "Accept: application/json"
    );

    curl_easy_setopt(
        curl,
        CURLOPT_URL,
        url.c_str()
    );

    curl_easy_setopt(
        curl,
        CURLOPT_HTTPHEADER,
        headers
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        write_callback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &response
    );

    curl_easy_setopt(
        curl,
        CURLOPT_FOLLOWLOCATION,
        1L
    );

    CURLcode result =
        curl_easy_perform(curl);

    if (result != CURLE_OK) {

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        throw std::runtime_error(
            std::string("curl error: ") +
            curl_easy_strerror(result)
        );
    }

    long status_code = 0;

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &status_code
    );

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (status_code < 200 || status_code >= 300) {

        throw std::runtime_error(
            "Alpaca returned HTTP " +
            std::to_string(status_code) +
            "\nResponse: " +
            response
        );
    }

    return response;
}

std::vector<Bar> AlpacaClient::get_bars(
    const std::vector<std::string>& symbols,
    const std::string& timeframe,
    const std::string& start,
    const std::string& end
) {
    std::vector<Bar> result;

    if (symbols.empty()) {
        return result;
    }

    std::ostringstream symbol_list;

    for (size_t i = 0; i < symbols.size(); ++i) {

        if (i > 0) {
            symbol_list << ",";
        }

        symbol_list << symbols[i];
    }

    std::string page_token;

    do {

        std::ostringstream url;

        url
            << "https://data.alpaca.markets/v2/stocks/bars"
            << "?symbols=" << symbol_list.str()
            << "&timeframe=" << timeframe
            << "&start=" << start
            << "&end=" << end
            << "&limit=10000"
            << "&sort=asc";

        if (!page_token.empty()) {
            url << "&page_token=" << page_token;
        }

        std::cout
            << "Requesting data";

        if (!page_token.empty()) {
            std::cout << " (next page)";
        }

        std::cout << "...\n";

        const std::string body =
            request(url.str());

        json data;

        try {
            data = json::parse(body);
        }
        catch (const json::parse_error& e) {
            throw std::runtime_error(
                std::string(
                    "Failed to parse Alpaca response: "
                ) + e.what()
            );
        }

        if (!data.contains("bars") ||
            data["bars"].is_null()) {

            break;
        }

        const auto& bars = data["bars"];

        if (!bars.is_object()) {
            throw std::runtime_error(
                "Unexpected 'bars' format"
            );
        }

        for (auto& [symbol, symbol_bars] :
             bars.items()) {

            if (symbol_bars.is_null() ||
                !symbol_bars.is_array()) {
                continue;
            }

            for (const auto& item :
                 symbol_bars) {

                if (!item.is_object()) {
                    continue;
                }

                Bar bar;

                bar.symbol = symbol;

                // Timestamp
                if (item.contains("t") &&
                    !item["t"].is_null() &&
                    item["t"].is_string()) {

                    bar.timestamp =
                        item["t"].get<std::string>();
                }

                // Open
                if (item.contains("o") &&
                    !item["o"].is_null() &&
                    item["o"].is_number()) {

                    bar.open =
                        item["o"].get<double>();
                }

                // High
                if (item.contains("h") &&
                    !item["h"].is_null() &&
                    item["h"].is_number()) {

                    bar.high =
                        item["h"].get<double>();
                }

                // Low
                if (item.contains("l") &&
                    !item["l"].is_null() &&
                    item["l"].is_number()) {

                    bar.low =
                        item["l"].get<double>();
                }

                // Close
                if (item.contains("c") &&
                    !item["c"].is_null() &&
                    item["c"].is_number()) {

                    bar.close =
                        item["c"].get<double>();
                }

                // Volume
                if (item.contains("v") &&
                    !item["v"].is_null() &&
                    item["v"].is_number()) {

                    bar.volume =
                        item["v"].get<double>();
                }

                // VWAP
                if (item.contains("vw") &&
                    !item["vw"].is_null() &&
                    item["vw"].is_number()) {

                    bar.vwap =
                        item["vw"].get<double>();
                }

                // Number of trades
                if (item.contains("n") &&
                    !item["n"].is_null() &&
                    item["n"].is_number()) {

                    bar.trade_count =
                        item["n"].get<long long>();
                }

                result.push_back(
                    std::move(bar)
                );
            }
        }

        // Safely handle null next_page_token
        page_token.clear();

        if (data.contains("next_page_token") &&
            !data["next_page_token"].is_null() &&
            data["next_page_token"].is_string()) {

            page_token =
                data["next_page_token"]
                    .get<std::string>();
        }

    } while (!page_token.empty());

    return result;
}
