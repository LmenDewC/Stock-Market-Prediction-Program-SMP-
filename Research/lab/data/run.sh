#!/bin/bash

if [ -z "$MONTH" ]; then
    echo -e "MONTH is not defined"
    echo 'run "export MONTH=<month>"'
    exit 1
fi

# Install dependencies
if dpkg -s nlohmann-json3-dev >/dev/null 2>&1; then
    echo "Package already is installed"
else
    sudo apt install nlohmann-json3-dev -y
fi

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
cmake --build . -j"$(nproc)"

# Run collector
./alpaca_collector

# Store collected data
MONTH_TXT="$MONTH.csv"
cp market_data.csv "../../../Data/$MONTH_TXT"


# Return to project directory
cd ..

# Remove build directory
rm -rf build