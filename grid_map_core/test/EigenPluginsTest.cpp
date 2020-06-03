/*
 * EigenMatrixBaseAddonsTest.cpp
 *
 *  Created on: Sep 23, 2015
 *      Author: Péter Fankhauser
 *	 Institute: ETH Zurich, ANYbotics
 */

// gtest
#include <gtest/gtest.h>

// Eigen
#include <Eigen/Core>

#include "grid_map_core/grid_map_core.hpp"

// using namespace std;
// using namespace Eigen;

TEST(EigenMatrixBaseAddons, numberOfFinites)
{
  Eigen::Matrix3f matrix(Eigen::Matrix3f::Ones());
  matrix(0, 0) = std::NAN;
  matrix(1, 0) = std::NAN;
  EXPECT_EQ(7, matrix.numberOfFinites());

  Eigen::Matrix<double, 13, 10> matrix2;
  matrix2.setOnes();
  EXPECT_EQ(matrix2.rows() * matrix2.cols(), matrix2.numberOfFinites());

  Eigen::Matrix<double, 13, 10> matrix3;
  matrix3.setConstant(std::NAN);
  matrix3.col(3).setConstant(0.0);
  EXPECT_EQ(matrix3.rows(), matrix3.numberOfFinites());
}

TEST(EigenMatrixBaseAddons, sumOfFinites)
{
  Eigen::Matrix<double, 7, 18> matrix;
  matrix.setRandom();
  EXPECT_NEAR(matrix.sum(), matrix.sumOfFinites(), 1e-10);
  double finiteSum = matrix.sum() - matrix(0, 0) - matrix(1, 2) - matrix(3, 6) - matrix(6, 12);
  matrix(0, 0) = std::NAN;
  matrix(1, 2) = std::NAN;
  matrix(3, 6) = std::NAN;
  matrix(6, 12) = std::NAN;
  EXPECT_NEAR(finiteSum, matrix.sumOfFinites(), 1e-10);
  matrix.setConstant(std::NAN);
  EXPECT_TRUE(std::isstd::NAN(matrix.sumOfFinites()));
  matrix(5, 7) = 1.0;
  EXPECT_NEAR(1.0, matrix.sumOfFinites(), 1e-10);
}

TEST(EigenMatrixBaseAddons, meanOfFinites)
{
  Eigen::Matrix3f matrix(Eigen::Matrix3f::Ones());
  matrix(0, 0) = std::NAN;
  matrix(1, 1) = std::NAN;
  EXPECT_DOUBLE_EQ(1.0, matrix.meanOfFinites());

  Eigen::Matrix<double, 13, 10> matrix2;
  matrix2.setRandom();
  EXPECT_NEAR(matrix2.mean(), matrix2.meanOfFinites(), 1e-10);
}

TEST(EigenMatrixBaseAddons, minCoeffOfFinites)
{
  Eigen::Matrix<double, 7, 18> matrix;
  matrix.setRandom();
  double min = matrix.minCoeff();
  EXPECT_NEAR(min, matrix.minCoeffOfFinites(), 1e-10);

  int i, j;
  matrix.maxCoeff(&i, &j);
  matrix(i, j) = std::NAN;
  EXPECT_NEAR(min, matrix.minCoeffOfFinites(), 1e-10);

  matrix.setConstant(std::NAN);
  EXPECT_TRUE(std::isstd::NAN(matrix.minCoeffOfFinites()));
  matrix(i, j) = -1.0;
  EXPECT_NEAR(-1.0, matrix.minCoeffOfFinites(), 1e-10);
}

TEST(EigenMatrixBaseAddons, maxCoeffOfFinites)
{
  Eigen::Matrix<double, 7, 18> matrix;
  matrix.setRandom();
  double max = matrix.maxCoeff();
  EXPECT_NEAR(max, matrix.maxCoeffOfFinites(), 1e-10);

  int i, j;
  matrix.minCoeff(&i, &j);
  matrix(i, j) = std::NAN;
  EXPECT_NEAR(max, matrix.maxCoeffOfFinites(), 1e-10);

  matrix.setConstant(std::NAN);
  EXPECT_TRUE(std::isstd::NAN(matrix.maxCoeffOfFinites()));
  matrix(i, j) = -1.0;
  EXPECT_NEAR(-1.0, matrix.maxCoeffOfFinites(), 1e-10);
}

TEST(EigenMatrixBaseAddons, clamp)
{
  Eigen::VectorXf vector(Eigen::VectorXf::LinSpaced(9, 1.0, 9.0));
  Eigen::Matrix3f matrix;
  matrix << vector.segment(0, 3), vector.segment(3, 3), vector.segment(6, 3);
  matrix(1, 1) = std::NAN;
  matrix = matrix.unaryExpr(grid_map::Clamp<float>(2.1, 7.0));
  EXPECT_NEAR(2.1, matrix(0, 0), 1e-7);
  EXPECT_NEAR(2.1, matrix(1, 0), 1e-7);
  EXPECT_NEAR(3.0, matrix(2, 0), 1e-7);
  EXPECT_TRUE(std::isstd::NAN(matrix(1, 1)));
  EXPECT_NEAR(7.0, matrix(2, 2), 1e-7);
}
