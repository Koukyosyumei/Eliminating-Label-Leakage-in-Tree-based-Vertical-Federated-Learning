#pragma once
#include <vector>
#include <iterator>
#include <limits>
#include <iostream>
#include <cmath>
using namespace std;

/**
 * @brief Compute gain of a node from the gradients and hessians.
 *
 * @param left_grad
 * @param right_grad
 * @param left_hess
 * @param right_hess
 * @param gam
 * @param lam
 * @return float
 */
float inline xgboost_compute_gain(vector<float> left_grad, vector<float> right_grad,
                                  vector<float> left_hess, vector<float> right_hess,
                                  float gam, float lam)
{
    float left_gain = 0;
    float right_gain = 0;
    float base_gain = 0;

    for (int c = 0; c < left_grad.size(); c++)
    {
        left_gain += (left_grad[c] * left_grad[c]) / (left_hess[c] + lam);
        right_gain += (right_grad[c] * right_grad[c]) / (right_hess[c] + lam);
        base_gain += ((left_grad[c] + right_grad[c]) *
                      (left_grad[c] + right_grad[c]) / (left_hess[c] + right_hess[c] + lam));
    }

    return 0.5 * (left_gain + right_gain - base_gain) - gam;
}

/**
 * @brief Compute the weight of a node.
 *
 * @param row_count
 * @param gradient
 * @param hessian
 * @param idxs
 * @param lam
 * @return vector<float>
 */
vector<float> inline xgboost_compute_weight(int row_count,
                                            vector<vector<float>> &gradient, vector<vector<float>> &hessian,
                                            vector<int> &idxs, float lam)
{
    int grad_dim = gradient[0].size();
    vector<float> sum_grad(grad_dim, 0);
    vector<float> sum_hess(grad_dim, 0);
    vector<float> node_weigths(grad_dim, 0);
    for (int i = 0; i < row_count; i++)
    {
        for (int c = 0; c < grad_dim; c++)
        {
            sum_grad[c] += gradient[idxs[i]][c];
            sum_hess[c] += hessian[idxs[i]][c];
        }
    }

    for (int c = 0; c < grad_dim; c++)
    {
        node_weigths[c] = -1 * (sum_grad[c] / (sum_hess[c] + lam));
    }
    return node_weigths;
}

vector<float> inline xgboost_compute_weight_from_pointer(int row_count,
                                                         vector<vector<float>> *gradient,
                                                         vector<vector<float>> *hessian,
                                                         vector<int> &idxs, float lam)
{
    int grad_dim = gradient->at(0).size();
    vector<float> sum_grad(grad_dim, 0);
    vector<float> sum_hess(grad_dim, 0);
    vector<float> node_weigths(grad_dim, 0);
    for (int i = 0; i < row_count; i++)
    {
        for (int c = 0; c < grad_dim; c++)
        {
            sum_grad[c] += gradient->at(idxs[i]).at(c);
            sum_hess[c] += hessian->at(idxs[i]).at(c);
        }
    }

    for (int c = 0; c < grad_dim; c++)
    {
        node_weigths[c] = -1 * (sum_grad[c] / (sum_hess[c] + lam));
    }
    return node_weigths;
}
