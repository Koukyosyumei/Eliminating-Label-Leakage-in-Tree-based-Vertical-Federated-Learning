#pragma once
#include <iostream>
#include <limits>
#include <queue>
#include <vector>

#include "../randomforest/randomforest.h"
#include "../secureboost/secureboost.h"
#include "../secureforest/secureforest.h"
#include "../utils/dok.h"
#include "../xgboost/xgboost.h"
using namespace std;

/**
 * @brief Travase nodes while updating the adjacency matrix with constant
 * weight.
 *
 * @tparam NodeType NodeType The type of Node.
 * @param node The node where you want to start travasing
 * @param adj_mat The space adhacency matrix to be updated
 * @param weight The weight parameter
 * @param target_party_id The target party id
 * @param max_num_samples_in_a_chunk
 * @para edge_weight_between_chunks
 * @return true
 * @return false
 */
template <typename NodeType>
inline void travase_nodes_to_extract_adjacency_matrix(
    NodeType *node, int max_depth, SparseMatrixDOK<float> &adj_mat,
    float weight, int target_party_id, int max_num_samples_in_a_chunk,
    int edge_weight_between_chunks) {
  queue<NodeType *> que;
  que.push(node);
  NodeType *temp_node;
  int temp_idxs_size;
  while (!que.empty()) {
    temp_node = que.front();
    que.pop();

    if (temp_node->is_leaf_flag) {
      if (!temp_node->not_splitted_flag || target_party_id == -1) {
        temp_idxs_size = temp_node->idxs.size();
        if (temp_idxs_size < max_num_samples_in_a_chunk) {
          for (int i = 0; i < temp_idxs_size; i++) {
            for (int j = i + 1; j < temp_idxs_size; j++) {
              adj_mat.add(temp_node->idxs[i], temp_node->idxs[j], weight);
            }
          }
        } else {
          int start_idx = 0;
          int end_idx = 0;
          while (start_idx < temp_idxs_size) {
            if (start_idx != 0) {
              adj_mat.add(temp_node->idxs[start_idx], temp_node->idxs[end_idx],
                          edge_weight_between_chunks);
            }
            end_idx =
                min(start_idx + max_num_samples_in_a_chunk, temp_idxs_size);
            for (int i = start_idx; i < end_idx; i++) {
              for (int j = i + 1; j < end_idx; j++) {
                adj_mat.add(temp_node->idxs[i], temp_node->idxs[j], weight);
              }
            }
            start_idx = end_idx;
          }
        }

        adj_mat.node_counter += 1;
        if (temp_idxs_size == 1) {
          adj_mat.zero_node_counter += 1;
        }
      }
    } else {
      bool not_splitted_flag = temp_node->left->not_splitted_flag &&
                               temp_node->right->not_splitted_flag;
      bool lmir_exclude_flag =
          temp_node->left->lmir_flag_exclude_passive_parties &&
          temp_node->right->lmir_flag_exclude_passive_parties;
      bool exclude_flag =
          (not_splitted_flag || lmir_exclude_flag) && (target_party_id != -1);

      if (exclude_flag) {
        temp_idxs_size = temp_node->idxs.size();

        if (temp_idxs_size < max_num_samples_in_a_chunk) {
          for (int i = 0; i < temp_idxs_size; i++) {
            for (int j = i + 1; j < temp_idxs_size; j++) {
              adj_mat.add(temp_node->idxs[i], temp_node->idxs[j], weight);
            }
          }
        } else {
          int start_idx = 0;
          int end_idx = 0;
          while (start_idx < temp_idxs_size) {
            if (start_idx != 0) {
              adj_mat.add(temp_node->idxs[start_idx], temp_node->idxs[end_idx],
                          edge_weight_between_chunks);
            }
            end_idx =
                min(start_idx + max_num_samples_in_a_chunk, temp_idxs_size);
            for (int i = start_idx; i < end_idx; i++) {
              for (int j = i + 1; j < end_idx; j++) {
                adj_mat.add(temp_node->idxs[i], temp_node->idxs[j], weight);
              }
            }
            start_idx = end_idx;
          }
        }
        adj_mat.node_counter += 1;
        if (temp_idxs_size == 1) {
          adj_mat.zero_node_counter += 1;
        }
      }

      if (!temp_node->left->lmir_flag_exclude_passive_parties ||
          !temp_node->right->lmir_flag_exclude_passive_parties) {
        que.push(temp_node->left);
        que.push(temp_node->right);
      }
    }

    // temp_node->idxs.clear();
    // temp_node->idxs.shrink_to_fit();
    // temp_node->val.clear();
    // temp_node->val.shrink_to_fit();
  }
}

/**
 * @brief Travase nodes while updating the adjacency matrix with constant
 * weight.
 *
 * @tparam NodeType NodeType The type of Node.
 * @param node The node where you want to start travasing
 * @param adj_mat The space adhacency matrix to be updated
 * @param weight The weight parameter
 * @param target_party_id The target party id
 * @return true
 * @return false
 */
template <typename NodeType>
inline void travase_nodes_to_extract_adjacency_matrix_as_freerider(
    NodeType *node, int max_depth, SparseMatrixDOK<float> &adj_mat,
    float weight, int target_party_id) {
  queue<NodeType *> que;
  que.push(node);
  NodeType *temp_node;
  int temp_idxs_size;
  while (!que.empty()) {
    // skip_flag = false;
    temp_node = que.front();
    que.pop();

    if (temp_node->is_leaf_flag) {
      temp_idxs_size = temp_node->idxs.size();
      for (int i = 0; i < temp_idxs_size; i++) {
        for (int j = i + 1; j < temp_idxs_size; j++) {
          adj_mat.add(temp_node->idxs[i], temp_node->idxs[j], weight);
        }
      }
    } else {
      if (!temp_node->left->is_leaf_flag && !temp_node->right->is_leaf_flag) {
        que.push(temp_node->left);
        que.push(temp_node->right);
      } else if (!temp_node->left->is_leaf_flag &&
                 temp_node->right->is_leaf_flag) {
        temp_idxs_size = temp_node->right->idxs.size();
        for (int i = 0; i < temp_idxs_size; i++) {
          for (int j = i + 1; j < temp_idxs_size; j++) {
            adj_mat.add(temp_node->right->idxs[i], temp_node->right->idxs[j],
                        weight);
          }
        }
        que.push(temp_node->left);
      } else if (temp_node->left->is_leaf_flag &&
                 !temp_node->right->is_leaf_flag) {
        temp_idxs_size = temp_node->left->idxs.size();
        for (int i = 0; i < temp_idxs_size; i++) {
          for (int j = i + 1; j < temp_idxs_size; j++) {
            adj_mat.add(temp_node->left->idxs[i], temp_node->left->idxs[j],
                        weight);
          }
        }
        que.push(temp_node->right);
      } else {
        temp_idxs_size = temp_node->idxs.size();
        for (int i = 0; i < temp_idxs_size; i++) {
          for (int j = i + 1; j < temp_idxs_size; j++) {
            adj_mat.add(temp_node->idxs[i], temp_node->idxs[j], weight);
          }
        }
      }
    }

    temp_node->idxs.clear();
    temp_node->idxs.shrink_to_fit();
    temp_node->val.clear();
    temp_node->val.shrink_to_fit();
  }
}

/**
 * @brief Update adjacency matrix with given tree.
 *
 * @param tree The tree to be transformed to a graph.
 * @param is_freerider Freerider flag
 * @param adj_mat The adjancecy matrix to be updated.
 * @param weight The weight parameter.
 * @param target_party_id The target party id.
 */
inline void extract_adjacency_matrix_from_tree(
    XGBoostTree *tree, bool is_freerider, SparseMatrixDOK<float> &adj_mat,
    float weight, int target_party_id, int max_num_samples_in_a_chunk,
    int edge_weight_between_chunks) {
  if (is_freerider) {
    travase_nodes_to_extract_adjacency_matrix_as_freerider<XGBoostNode>(
        &tree->dtree, tree->dtree.depth, adj_mat, weight, target_party_id);
  } else {
    travase_nodes_to_extract_adjacency_matrix<XGBoostNode>(
        &tree->dtree, tree->dtree.depth, adj_mat, weight, target_party_id,
        max_num_samples_in_a_chunk, edge_weight_between_chunks);
  }
}

/**
 * @brief Update adjacency matrix with given tree.
 *
 * @param tree The tree to be transformed to a graph.
 * @param is_freerider Freerider flag
 * @param adj_mat The adjancecy matrix to be updated.
 * @param weight The weight parameter.
 * @param target_party_id The target party id.
 */
inline void extract_adjacency_matrix_from_tree(
    SecureBoostTree *tree, bool is_freerider, SparseMatrixDOK<float> &adj_mat,
    float weight, int target_party_id, int max_num_samples_in_a_chunk,
    int edge_weight_between_chunks) {
  if (is_freerider) {
    travase_nodes_to_extract_adjacency_matrix_as_freerider<SecureBoostNode>(
        &tree->dtree, tree->dtree.depth, adj_mat, weight, target_party_id);
  } else {
    travase_nodes_to_extract_adjacency_matrix<SecureBoostNode>(
        &tree->dtree, tree->dtree.depth, adj_mat, weight, target_party_id,
        max_num_samples_in_a_chunk, edge_weight_between_chunks);
  }
}

inline void extract_adjacency_matrix_from_tree(
    SecureForestTree *tree, bool is_freerider, SparseMatrixDOK<float> &adj_mat,
    float weight, int target_party_id, int max_num_samples_in_a_chunk,
    int edge_weight_between_chunks) {
  if (is_freerider) {
    travase_nodes_to_extract_adjacency_matrix_as_freerider<SecureForestNode>(
        &tree->dtree, tree->dtree.depth, adj_mat, weight, target_party_id);
  } else {
    travase_nodes_to_extract_adjacency_matrix<SecureForestNode>(
        &tree->dtree, tree->dtree.depth, adj_mat, weight, target_party_id,
        max_num_samples_in_a_chunk, edge_weight_between_chunks);
  }
}

/**
 * @brief Update adjacency matrix with given tree.
 *
 * @param tree The tree to be transformed to a graph.
 * @param is_freerider Freerider flag
 * @param adj_mat The adjancecy matrix to be updated.
 * @param weight The weight parameter.
 * @param target_party_id The target party id.
 */
inline void extract_adjacency_matrix_from_tree(
    RandomForestTree *tree, bool is_freerider, SparseMatrixDOK<float> &adj_mat,
    float weight, int target_party_id, int max_num_samples_in_a_chunk,
    int edge_weight_between_chunks) {
  if (is_freerider) {
    travase_nodes_to_extract_adjacency_matrix_as_freerider<RandomForestNode>(
        &tree->dtree, tree->dtree.depth, adj_mat, weight, target_party_id);
  } else {
    travase_nodes_to_extract_adjacency_matrix<RandomForestNode>(
        &tree->dtree, tree->dtree.depth, adj_mat, weight, target_party_id,
        max_num_samples_in_a_chunk, edge_weight_between_chunks);
  }
}

/**
 * @brief Extract adjacency matrix from the trained model
 *
 * @param model The target tree-based model
 * @param is_freerider Freerider flag
 * @param target_party_id The target party id. He cannot observe the leaf split
 * information.
 * @param skip_round The number of skipped rounds.
 * @param eta The discount factor.
 * @return SparseMatrixDOK<float>
 */
inline SparseMatrixDOK<float>
extract_adjacency_matrix_from_forest(XGBoostBase *model, bool is_freerider,
                                     int target_party_id = -1,
                                     int skip_round = 0, float eta = 0.3,
                                     int max_num_samples_in_a_chunk = 1000000,
                                     int edge_weight_between_chunks = 100) {
  int num_row = model->estimators[0].num_row;
  SparseMatrixDOK<float> adj_matrix(num_row, num_row, 0.0, true, true);
  for (int i = 0; i < model->estimators.size(); i++) {
    if (i >= skip_round) {
      extract_adjacency_matrix_from_tree(
          &model->estimators[i], is_freerider, adj_matrix,
          pow(eta, float(i - skip_round)), target_party_id,
          max_num_samples_in_a_chunk, edge_weight_between_chunks);
    }
  }
  return adj_matrix;
}

inline SparseMatrixDOK<float> extract_adjacency_matrix_from_forest(
    SecureForestClassifier *model, bool is_freerider, int target_party_id = -1,
    int skip_round = 0, int max_num_samples_in_a_chunk = 1000000,
    int edge_weight_between_chunks = 100) {
  int num_row = model->estimators[0].num_row;
  SparseMatrixDOK<float> adj_matrix(num_row, num_row, 0.0, true, true);
  for (int i = 0; i < model->estimators.size(); i++) {
    if (i >= skip_round) {
      extract_adjacency_matrix_from_tree(
          &model->estimators[i], is_freerider, adj_matrix, 1.0, target_party_id,
          max_num_samples_in_a_chunk, edge_weight_between_chunks);
    }
  }
  return adj_matrix;
}

/**
 * @brief Extract adjacency matrix from the trained model
 *
 * @param model The target tree-based model
 * @param is_freerider Freerider flag
 * @param target_party_id The target party id. He cannot observe the leaf split
 * information.
 * @param skip_round The number of skipped rounds.
 * @param eta The discount factor.
 * @return SparseMatrixDOK<float>
 */
inline SparseMatrixDOK<float>
extract_adjacency_matrix_from_forest(SecureBoostBase *model, bool is_freerider,
                                     int target_party_id = -1,
                                     int skip_round = 0, float eta = 0.3,
                                     int max_num_samples_in_a_chunk = 1000000,
                                     int edge_weight_between_chunks = 100) {
  int num_row = model->estimators[0].num_row;
  SparseMatrixDOK<float> adj_matrix(num_row, num_row, 0.0, true, true);
  for (int i = 0; i < model->estimators.size(); i++) {
    if (i >= skip_round) {
      extract_adjacency_matrix_from_tree(
          &model->estimators[i], is_freerider, adj_matrix,
          pow(eta, float(i - skip_round)), target_party_id,
          max_num_samples_in_a_chunk, edge_weight_between_chunks);
    }
  }
  return adj_matrix;
}

/**
 * @brief Extract adjacency matrix from the trained model
 *
 * @param model The target tree-based model
 * @param is_freerider Freerider flag
 * @param target_party_id The target party id. He cannot observe the leaf split
 * information.
 * @param skip_round The number of skipped rounds.
 * @return SparseMatrixDOK<float>
 */
inline SparseMatrixDOK<float> extract_adjacency_matrix_from_forest(
    RandomForestClassifier *model, bool is_freerider, int target_party_id = -1,
    int skip_round = 0, int max_num_samples_in_a_chunk = 1000000,
    int edge_weight_between_chunks = 100) {
  int num_row = model->estimators[0].num_row;
  SparseMatrixDOK<float> adj_matrix(num_row, num_row, 0.0, true, true);
  for (int i = 0; i < model->estimators.size(); i++) {
    if (i >= skip_round) {
      extract_adjacency_matrix_from_tree(
          &model->estimators[i], is_freerider, adj_matrix, 1.0, target_party_id,
          max_num_samples_in_a_chunk, edge_weight_between_chunks);
    }
  }
  return adj_matrix;
}
