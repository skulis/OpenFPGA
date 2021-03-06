/******************************************************************************
 * This file includes member functions for data structure BitstreamManager 
 ******************************************************************************/
#include <algorithm>

#include "vtr_assert.h"
#include "bitstream_manager.h"

/**************************************************
 * Public Accessors : Aggregates
 *************************************************/
/* Find all the configuration bits */
BitstreamManager::config_bit_range BitstreamManager::bits() const {
  return vtr::make_range(bit_ids_.begin(), bit_ids_.end());
}

/* Find all the configuration blocks */
BitstreamManager::config_block_range BitstreamManager::blocks() const {
  return vtr::make_range(block_ids_.begin(), block_ids_.end());
}

/******************************************************************************
 * Public Accessors
 ******************************************************************************/
bool BitstreamManager::bit_value(const ConfigBitId& bit_id) const {
  /* Ensure a valid id */
  VTR_ASSERT(true == valid_bit_id(bit_id));

  return bit_values_[bit_id];
}

std::string BitstreamManager::block_name(const ConfigBlockId& block_id) const {
  /* Ensure the input ids are valid */
  VTR_ASSERT(true == valid_block_id(block_id));

  return block_names_[block_id];
}

ConfigBlockId BitstreamManager::block_parent(const ConfigBlockId& block_id) const {
  /* Ensure the input ids are valid */
  VTR_ASSERT(true == valid_block_id(block_id));

  return parent_block_ids_[block_id];
}

std::vector<ConfigBlockId> BitstreamManager::block_children(const ConfigBlockId& block_id) const {
  /* Ensure the input ids are valid */
  VTR_ASSERT(true == valid_block_id(block_id));

  return child_block_ids_[block_id];
}

std::vector<ConfigBitId> BitstreamManager::block_bits(const ConfigBlockId& block_id) const {
  /* Ensure the input ids are valid */
  VTR_ASSERT(true == valid_block_id(block_id));

  return block_bit_ids_[block_id];
}

ConfigBlockId BitstreamManager::bit_parent_block(const ConfigBitId& bit_id) const {
  /* Ensure the input ids are valid */
  VTR_ASSERT(true == valid_bit_id(bit_id));

  return bit_parent_block_ids_[bit_id];
}

size_t BitstreamManager::bit_index_in_parent_block(const ConfigBitId& bit_id) const {
  /* Ensure the input ids are valid */
  VTR_ASSERT(true == valid_bit_id(bit_id));

  ConfigBlockId bit_parent_block = bit_parent_block_ids_[bit_id];

  VTR_ASSERT(true == valid_block_id(bit_parent_block));

  for (size_t index = 0; index < block_bits(bit_parent_block).size(); ++index) {
    if (bit_id == block_bits(bit_parent_block)[index]) {
      return index;
    }
  }

  /* Not found, return in valid value */
  return size_t(-1); 
}

/* Find the child block in a bitstream manager with a given name */
ConfigBlockId BitstreamManager::find_child_block(const ConfigBlockId& block_id, 
                                                 const std::string& child_block_name) const {
  /* Ensure the input ids are valid */
  VTR_ASSERT(true == valid_block_id(block_id));

  std::vector<ConfigBlockId> candidates;

  for (const ConfigBlockId& child : block_children(block_id)) {
    if (0 == child_block_name.compare(block_name(child))) {
      candidates.push_back(child);
    }
  }

  /* We should have 0 or 1 candidate! */
  VTR_ASSERT(0 == candidates.size() || 1 == candidates.size());
  if (0 == candidates.size()) {
    /* Not found, return an invalid value */
    return ConfigBlockId::INVALID();
  }
  return candidates[0];
}

/******************************************************************************
 * Public Mutators
 ******************************************************************************/
ConfigBitId BitstreamManager::add_bit(const bool& bit_value) {
  ConfigBitId bit = ConfigBitId(bit_ids_.size());
  /* Add a new bit, and allocate associated data structures */
  bit_ids_.push_back(bit);
  bit_values_.push_back(bit_value);
  shared_config_bit_values_.emplace_back();
  bit_parent_block_ids_.push_back(ConfigBlockId::INVALID());

  return bit; 
}

ConfigBlockId BitstreamManager::add_block(const std::string& block_name) {
  ConfigBlockId block = ConfigBlockId(block_ids_.size());
  /* Add a new bit, and allocate associated data structures */
  block_ids_.push_back(block);
  block_names_.push_back(block_name);
  block_bit_ids_.emplace_back();
  parent_block_ids_.push_back(ConfigBlockId::INVALID());
  child_block_ids_.emplace_back();

  return block; 
}

void BitstreamManager::add_child_block(const ConfigBlockId& parent_block, const ConfigBlockId& child_block) {
  /* Ensure the input ids are valid */
  VTR_ASSERT(true == valid_block_id(parent_block));
  VTR_ASSERT(true == valid_block_id(child_block));

  /* We should have only a parent block for each block! */
  VTR_ASSERT(ConfigBlockId::INVALID() == parent_block_ids_[child_block]);

  /* Ensure the child block is not in the list of children of the parent block */
  std::vector<ConfigBlockId>::iterator it = std::find(child_block_ids_[parent_block].begin(), child_block_ids_[parent_block].end(), child_block);
  VTR_ASSERT(it == child_block_ids_[parent_block].end());

  /* Add the child_block to the parent_block */
  child_block_ids_[parent_block].push_back(child_block);
  /* Register the block in the parent of the block */
  parent_block_ids_[child_block] = parent_block;
}

void BitstreamManager::add_bit_to_block(const ConfigBlockId& block, const ConfigBitId& bit) {
  /* Ensure the input ids are valid */
  VTR_ASSERT(true == valid_block_id(block));
  VTR_ASSERT(true == valid_bit_id(bit));

  /* We should have only a parent block for each bit! */
  VTR_ASSERT(ConfigBlockId::INVALID() == bit_parent_block_ids_[bit]);

  /* Add the bit to the block */
  block_bit_ids_[block].push_back(bit);
  /* Register the block in the parent of the bit */
  bit_parent_block_ids_[bit] = block;
}

void BitstreamManager::add_shared_config_bit_values(const ConfigBitId& bit, const std::vector<bool>& shared_config_bits) {
  /* Ensure the input ids are valid */
  VTR_ASSERT(true == valid_bit_id(bit));
 
  shared_config_bit_values_[bit] = shared_config_bits;
}

/******************************************************************************
 * Public Validators
 ******************************************************************************/
bool BitstreamManager::valid_bit_id(const ConfigBitId& bit_id) const {
  return (size_t(bit_id) < bit_ids_.size()) && (bit_id == bit_ids_[bit_id]);
}

bool BitstreamManager::valid_block_id(const ConfigBlockId& block_id) const {
  return (size_t(block_id) < block_ids_.size()) && (block_id == block_ids_[block_id]);
}


