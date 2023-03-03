#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client.hpp>

// Chunk size for Merkle tree chunking (4MB)
static const size_t CHUNK_SIZE = 4 * 1024 * 1024;

// Get the Merkle root of a list of chunks
bc::hash_digest get_merkle_root(const std::vector<bc::data_chunk>& chunks) {
  // Build the Merkle tree
  std::vector<bc::hash_digest> tree(chunks.size() * 2);
  for (size_t i = 0; i < chunks.size(); ++i) {
    bc::hash_digest hash = bc::hash_literal(chunks[i].data(), chunks[i].size());
    tree[i + chunks.size()] = hash;
  }
  for (int i = chunks.size() - 1; i > 0; --i) {
    bc::hash_digest left = tree[i * 2];
    bc::hash_digest right = tree[i * 2 + 1];
    bc::data_chunk concat(left.begin(), left.end());
    concat.insert(concat.end(), right.begin(), right.end());
    tree[i] = bc::hash_literal(concat.data(), concat.size());
  }

  // Return the Merkle root
  return tree[1];
}

int main() {
  // Connect to Bitcoin Core
  bc::config::endpoint endpoint("127.0.0.1", "8332");
  bc::client::obelisk_client client(endpoint);

  // Open the file for reading
  std::ifstream file("large_file.txt", std::ios::binary);
  if (!file) {
    std::cerr << "Error: could not open file for reading." << std::endl;
    return 1;
  }

  // Read the file in chunks and add them to a vector
  std::vector<bc::data_chunk> chunks;
  while (!file.eof()) {
    char buffer[CHUNK_SIZE];
    file.read(buffer, CHUNK_SIZE);
    size_t count = file.gcount();
    bc::data_chunk chunk(buffer, buffer + count);
    chunks.push_back(chunk);
  }

  // Get the Merkle root of the chunks
  bc::hash_digest merkle_root = get_merkle_root(chunks);

  // Output the Merkle root
  std::cout << "Merkle root: " << bc::encode_base16(merkle_root) << std::endl;

  return 0;
}
