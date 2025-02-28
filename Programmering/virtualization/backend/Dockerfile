FROM ubuntu:latest

# Nødvendige installasjoner og pakker
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    libcpprest-dev \
    libssl-dev \
    libboost-all-dev \
    python3 \
    python3-pip

WORKDIR /app
COPY . .

RUN g++ -std=c++14 app.cpp -o app -lcpprest -lboost_system -lcrypto -lssl -pthread

EXPOSE 8080

CMD ["./app"]