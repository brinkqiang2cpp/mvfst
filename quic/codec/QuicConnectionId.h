/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 */

#pragma once

#include <folly/Optional.h>

#include <folly/String.h>
#include <folly/hash/Hash.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include <array>

#include <quic/QuicConstants.h>

namespace quic {
constexpr uint8_t kStatelessResetTokenLength = 16;
using StatelessResetToken = std::array<uint8_t, kStatelessResetTokenLength>;

// max size of a connId as specified in the draft
constexpr size_t kMaxConnectionIdSize = 20;

// Minimum required length (in bytes) for the destination connection-id
// on inbound initial packets.
constexpr size_t kMinInitialDestinationConnIdLength = 8;

// set conn id version at the first 4 bits
constexpr uint8_t kShortVersionId = 0x1;

constexpr uint64_t kInitialSequenceNumber = 0x0;

struct ConnectionId {
  uint8_t* data();

  const uint8_t* data() const;

  uint8_t size() const;

  explicit ConnectionId(const std::vector<uint8_t>& connidIn);

  explicit ConnectionId(folly::io::Cursor& cursor, size_t len);

  bool operator==(const ConnectionId& other) const;
  bool operator!=(const ConnectionId& other) const;

  std::string hex() const;

  /**
   * Create an connection without any checks for tests.
   */
  static ConnectionId createWithoutChecks(const std::vector<uint8_t>& connidIn);

  /**
   * Create a random ConnectionId with the given length.
   */
  static ConnectionId createRandom(size_t len);

 private:
  ConnectionId() = default;

  std::array<uint8_t, kMaxConnectionIdSize> connid;
  uint8_t connidLen;
};

struct ConnectionIdHash {
  size_t operator()(const ConnectionId& connId) const {
    return folly::hash::fnv32_buf(connId.data(), connId.size());
  }
};

inline std::ostream& operator<<(std::ostream& os, const ConnectionId& connId) {
  os << connId.hex();
  return os;
}

inline folly::IOBuf toData(const ConnectionId& connId) {
  return folly::IOBuf::wrapBufferAsValue(connId.data(), connId.size());
}

struct ConnectionIdData {
  ConnectionIdData(const ConnectionId& connIdIn, uint64_t sequenceNumberIn)
      : connId(connIdIn), sequenceNumber(sequenceNumberIn) {}

  ConnectionIdData(
      const ConnectionId& connIdIn,
      uint64_t sequenceNumberIn,
      StatelessResetToken tokenIn)
      : connId(connIdIn), sequenceNumber(sequenceNumberIn), token(tokenIn) {}

  ConnectionId connId;
  uint64_t sequenceNumber;
  folly::Optional<StatelessResetToken> token;
};

/**
 * Encapsulate parameters to generate server chosen connection id
 */
struct ServerConnectionIdParams {
  explicit ServerConnectionIdParams(
      uint16_t hostIdIn,
      uint8_t processIdIn,
      uint8_t workerIdIn)
      : ServerConnectionIdParams(
            kShortVersionId,
            hostIdIn,
            processIdIn,
            workerIdIn) {}

  explicit ServerConnectionIdParams(
      uint8_t versionIn,
      uint16_t hostIdIn,
      uint8_t processIdIn,
      uint8_t workerIdIn) {
    setVersion(versionIn);
    setHostId(hostIdIn);
    setProcessId(processIdIn);
    setWorkerId(workerIdIn);
  }

  /**
   * Set Quic connection-id short version
   */
  void setVersion(uint8_t versionIn);

  /**
   * Set Quic Host id
   */
  void setHostId(uint16_t hostIdIn);

  /**
   * Set Quic process id
   */
  void setProcessId(uint8_t processIdIn);

  /**
   * Set Quic server worker Id
   */
  void setWorkerId(uint8_t workerIdIn);

  // Quic connection-id short version
  uint8_t version{0};
  // Quic Host id
  uint16_t hostId{0};
  // Quic process id
  uint8_t processId{0};
  // Quic server worker Id
  uint8_t workerId{0};
};

bool operator==(
    const ServerConnectionIdParams& lhs,
    const ServerConnectionIdParams& rhs);

bool operator!=(
    const ServerConnectionIdParams& lhs,
    const ServerConnectionIdParams& rhs);

} // namespace quic
