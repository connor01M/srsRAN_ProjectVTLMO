/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "udp_network_gateway.h"
#include "srsgnb/adt/span.h"
#include "srsgnb/srslog/srslog.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace srsgnb;

udp_network_gateway::udp_network_gateway(network_gateway_config         config_,
                                         network_gateway_data_notifier& data_notifier_) :
  config(config_), data_notifier(data_notifier_), logger(srslog::fetch_basic_logger("UDP-NW-GW"))
{
}

bool udp_network_gateway::is_initialized()
{
  return sock_fd != -1;
}

void udp_network_gateway::handle_pdu(const byte_buffer& pdu)
{
  logger.debug("Sending PDU of {} bytes", pdu.length());

  if (not is_initialized()) {
    logger.error("Socket not initialized");
    return;
  }

  if (pdu.length() > network_gateway_udp_max_len) {
    logger.error("PDU of {} bytes exceeds maximum length of {} bytes", pdu.length(), network_gateway_udp_max_len);
    return;
  }

  // Fixme: consider class member on heap when sequential access is guaranteed
  std::array<uint8_t, network_gateway_udp_max_len> tmp_mem; // no init

  span<const uint8_t> pdu_span = to_span(pdu, tmp_mem);

  int bytes_sent = send(sock_fd, pdu_span.data(), pdu_span.size_bytes(), 0);
  if (bytes_sent == -1) {
    logger.error("Couldn't send {} B of data on UDP socket: {}", pdu_span.size_bytes(), strerror(errno));
    return;
  }
}

bool udp_network_gateway::create_and_bind()
{
  struct addrinfo hints;
  // support ipv4, ipv6 and hostnames
  hints.ai_family    = AF_UNSPEC;
  hints.ai_socktype  = SOCK_DGRAM;
  hints.ai_flags     = 0;
  hints.ai_protocol  = IPPROTO_UDP;
  hints.ai_canonname = nullptr;
  hints.ai_addr      = nullptr;
  hints.ai_next      = nullptr;

  std::string      bind_port = std::to_string(config.bind_port);
  struct addrinfo* results;

  int ret = getaddrinfo(config.bind_address.c_str(), bind_port.c_str(), &hints, &results);
  if (ret != 0) {
    logger.error("Getaddrinfo error: {} - {}", config.bind_address, gai_strerror(ret));
    return false;
  }

  struct addrinfo* result;
  for (result = results; result != nullptr; result = result->ai_next) {
    // create UDP socket
    sock_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock_fd == -1) {
      ret = errno;
      continue;
    }

    if (not set_sockopts()) {
      close_socket();
      continue;
    }

    char ip_addr[NI_MAXHOST], port_nr[NI_MAXSERV];
    getnameinfo(
        result->ai_addr, result->ai_addrlen, ip_addr, NI_MAXHOST, port_nr, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
    logger.debug("Binding to {} port {}", ip_addr, port_nr);

    if (bind(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
      // binding failed, try next address
      ret = errno;
      logger.debug("Failed to bind to {}:{} - {}", ip_addr, port_nr, strerror(ret));
      close_socket();
      continue;
    }

    // store client address
    memcpy(&client_addr, result->ai_addr, result->ai_addrlen);
    client_addrlen     = result->ai_addrlen;
    client_ai_family   = result->ai_family;
    client_ai_socktype = result->ai_socktype;
    client_ai_protocol = result->ai_protocol;

    // set socket to non-blocking after bind is successful
    if (config.non_blocking_mode) {
      if (not set_non_blocking()) {
        // failed, try next address
        logger.error("Socket not non-blocking");
        close_socket();
        continue;
      }
    }

    logger.debug("Binding successful");
    break;
  }

  freeaddrinfo(results);

  if (sock_fd == -1) {
    logger.error("Error binding to {}:{} - {}", config.bind_address, config.bind_port, strerror(ret));
    return false;
  }

  return true;
}

bool udp_network_gateway::listen()
{
  // UDP does not listen for connections
  logger.info("Ignoring unnecessary call of `listen()` on UDP gateway.");
  return true;
}

bool udp_network_gateway::create_and_connect()
{
  // bind to address/port
  if (not config.bind_address.empty()) {
    if (not create_and_bind()) {
      logger.error("Couldn't bind to address {}:{}", config.bind_address, config.bind_port);
      close_socket();
      return false;
    }
  }

  struct addrinfo hints;
  // support ipv4, ipv6 and hostnames
  hints.ai_family    = AF_UNSPEC;
  hints.ai_socktype  = SOCK_DGRAM;
  hints.ai_flags     = 0;
  hints.ai_protocol  = IPPROTO_UDP;
  hints.ai_canonname = nullptr;
  hints.ai_addr      = nullptr;
  hints.ai_next      = nullptr;

  std::string      connect_port = std::to_string(config.connect_port);
  struct addrinfo* results;

  int ret = getaddrinfo(config.connect_address.c_str(), connect_port.c_str(), &hints, &results);
  if (ret != 0) {
    logger.error("Getaddrinfo error: {} - {}", config.connect_address, gai_strerror(ret));
    return false;
  }

  struct addrinfo* result;
  for (result = results; result != nullptr; result = result->ai_next) {
    // create UDP socket
    sock_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock_fd == -1) {
      ret = errno;
      continue;
    }

    if (not set_sockopts()) {
      close_socket();
      continue;
    }

    char ip_addr[NI_MAXHOST], port_nr[NI_MAXSERV];
    getnameinfo(
        result->ai_addr, result->ai_addrlen, ip_addr, NI_MAXHOST, port_nr, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
    logger.debug("Connecting to {} port {}", ip_addr, port_nr);

    if (connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
      // connection failed, try next address
      ret = errno;
      logger.debug("Failed to connect to {}:{} - {}", ip_addr, port_nr, strerror(ret));
      close_socket();
      continue;
    }

    // store server address
    memcpy(&server_addr, result->ai_addr, result->ai_addrlen);
    server_addrlen     = result->ai_addrlen;
    server_ai_family   = result->ai_family;
    server_ai_socktype = result->ai_socktype;
    server_ai_protocol = result->ai_protocol;

    // set socket to non-blocking after connect is established
    if (config.non_blocking_mode) {
      if (not set_non_blocking()) {
        // failed, try next address
        logger.error("Socket not non-blocking");
        close_socket();
        continue;
      }
    }

    logger.info("Connection successful");
    break;
  }

  freeaddrinfo(results);

  if (sock_fd == -1) {
    logger.error("Error connecting to {}:{} - {}", config.connect_address, config.connect_port, strerror(ret));
    return false;
  }

  return true;
}

bool udp_network_gateway::recreate_and_reconnect()
{
  if (is_initialized()) {
    // Continue using existing socket
    logger.info("Called `recreate_and_reconnect()` on initialized UDP gateway: keep using previous connection.");
    return true;
  }

  logger.warning("Called `recreate_and_reconnect()` on uninitialized UDP gateway: establishing new connection.");
  return create_and_connect();
}

void udp_network_gateway::receive()
{
  // Fixme: consider class member on heap when sequential access is guaranteed
  std::array<uint8_t, network_gateway_udp_max_len> tmp_mem; // no init

  int rx_bytes = recv(sock_fd, tmp_mem.data(), network_gateway_udp_max_len, 0);

  if (rx_bytes == -1 && errno != EAGAIN) {
    logger.error("Error reading from UDP socket: {}", strerror(errno));
  } else if (rx_bytes == -1 && errno == EAGAIN) {
    logger.debug("Socket timeout reached");
  } else {
    logger.debug("Received {} bytes on UDP socket", rx_bytes);
    span<uint8_t> payload(tmp_mem.data(), rx_bytes);
    byte_buffer   pdu = {payload};
    data_notifier.on_new_pdu(std::move(pdu));
  }
}

int udp_network_gateway::get_socket_fd()
{
  return sock_fd;
}

bool udp_network_gateway::set_sockopts()
{
  if (config.rx_timeout_sec > 0) {
    if (not set_receive_timeout(config.rx_timeout_sec)) {
      logger.error("Couldn't set receive timeout for socket");

      return false;
    }
  }

  if (config.reuse_addr) {
    if (not set_reuse_addr()) {
      logger.error("Couldn't set reuseaddr for socket");
      return false;
    }
  }

  return true;
}

bool udp_network_gateway::set_non_blocking()
{
  int flags = fcntl(sock_fd, F_GETFL, 0);
  if (flags == -1) {
    logger.error("Error getting socket flags: {}", strerror(errno));
    return false;
  }

  int s = fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);
  if (s == -1) {
    logger.error("Error setting socket to non-blocking mode: {}", strerror(errno));
    return false;
  }

  return true;
}

bool udp_network_gateway::set_receive_timeout(unsigned rx_timeout_sec)
{
  struct timeval tv;
  tv.tv_sec  = rx_timeout_sec;
  tv.tv_usec = 0;

  if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv)) {
    logger.error("Couldn't set receive timeout for socket: {}", strerror(errno));
    return false;
  }

  return true;
}

bool udp_network_gateway::set_reuse_addr()
{
  int one = 1;
  if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
    logger.error("Couldn't set reuseaddr for socket: {}", strerror(errno));
    return false;
  }
  return true;
}

bool udp_network_gateway::close_socket()
{
  if (not is_initialized()) {
    logger.error("Socket not initialized");
    return false;
  }

  if (close(sock_fd) == -1) {
    logger.error("Error closing socket: {}", strerror(errno));
    return false;
  }

  sock_fd = -1;

  return true;
}