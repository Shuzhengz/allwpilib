// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include <limits>
#include <random>

#include "frc/estimator/MecanumDrivePoseEstimator.h"
#include "frc/geometry/Pose2d.h"
#include "frc/kinematics/MecanumDriveKinematics.h"
#include "frc/trajectory/TrajectoryGenerator.h"
#include "gtest/gtest.h"

TEST(MecanumDrivePoseEstimatorTest, AccuracyFacingTrajectory) {
  frc::MecanumDriveKinematics kinematics{
      frc::Translation2d{1_m, 1_m}, frc::Translation2d{1_m, -1_m},
      frc::Translation2d{-1_m, -1_m}, frc::Translation2d{-1_m, 1_m}};

  frc::MecanumDriveWheelPositions wheelPositions;

  frc::MecanumDrivePoseEstimator estimator{frc::Rotation2d{},
                                           frc::Pose2d{},
                                           wheelPositions,
                                           kinematics,
                                           {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1},
                                           {0.05, 0.05, 0.05, 0.05, 0.05},
                                           {0.1, 0.1, 0.1}};

  frc::Trajectory trajectory = frc::TrajectoryGenerator::GenerateTrajectory(
      std::vector{frc::Pose2d{0_m, 0_m, 45_deg}, frc::Pose2d{3_m, 0_m, -90_deg},
                  frc::Pose2d{0_m, 0_m, 135_deg},
                  frc::Pose2d{-3_m, 0_m, -90_deg},
                  frc::Pose2d{0_m, 0_m, 45_deg}},
      frc::TrajectoryConfig(5.0_mps, 2.0_mps_sq));

  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0.0, 1.0);

  units::second_t dt = 0.02_s;
  units::second_t t = 0_s;

  units::second_t kVisionUpdateRate = 0.1_s;
  frc::Pose2d lastVisionPose;
  units::second_t lastVisionUpdateTime{-std::numeric_limits<double>::max()};

  std::vector<frc::Pose2d> visionPoses;

  double maxError = -std::numeric_limits<double>::max();
  double errorSum = 0;

  while (t < trajectory.TotalTime()) {
    frc::Trajectory::State groundTruthState = trajectory.Sample(t);

    if (lastVisionUpdateTime + kVisionUpdateRate < t) {
      if (lastVisionPose != frc::Pose2d{}) {
        estimator.AddVisionMeasurement(lastVisionPose, lastVisionUpdateTime);
      }
      lastVisionPose =
          groundTruthState.pose +
          frc::Transform2d{
              frc::Translation2d{distribution(generator) * 0.1_m,
                                 distribution(generator) * 0.1_m},
              frc::Rotation2d{distribution(generator) * 0.1 * 1_rad}};
      visionPoses.push_back(lastVisionPose);
      lastVisionUpdateTime = t;
    }

    auto wheelSpeeds = kinematics.ToWheelSpeeds(
        {groundTruthState.velocity, 0_mps,
         groundTruthState.velocity * groundTruthState.curvature});

    wheelPositions.frontLeft += wheelSpeeds.frontLeft * dt;
    wheelPositions.frontRight += wheelSpeeds.frontRight * dt;
    wheelPositions.rearLeft += wheelSpeeds.rearLeft * dt;
    wheelPositions.rearRight += wheelSpeeds.rearRight * dt;

    auto xhat = estimator.UpdateWithTime(
        t,
        groundTruthState.pose.Rotation() +
            frc::Rotation2d{distribution(generator) * 0.05_rad},
        wheelSpeeds, wheelPositions);
    double error = groundTruthState.pose.Translation()
                       .Distance(xhat.Translation())
                       .value();

    if (error > maxError) {
      maxError = error;
    }
    errorSum += error;

    t += dt;
  }

  EXPECT_LT(errorSum / (trajectory.TotalTime().value() / dt.value()), 0.05);
  EXPECT_LT(maxError, 0.125);
}

TEST(MecanumDrivePoseEstimatorTest, AccuracyFacingXAxis) {
  frc::MecanumDriveKinematics kinematics{
      frc::Translation2d{1_m, 1_m}, frc::Translation2d{1_m, -1_m},
      frc::Translation2d{-1_m, -1_m}, frc::Translation2d{-1_m, 1_m}};

  frc::MecanumDriveWheelPositions wheelPositions;

  frc::MecanumDrivePoseEstimator estimator{frc::Rotation2d{},
                                           frc::Pose2d{},
                                           wheelPositions,
                                           kinematics,
                                           {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1},
                                           {0.05, 0.05, 0.05, 0.05, 0.05},
                                           {0.1, 0.1, 0.1}};

  frc::Trajectory trajectory = frc::TrajectoryGenerator::GenerateTrajectory(
      std::vector{frc::Pose2d{0_m, 0_m, 45_deg}, frc::Pose2d{3_m, 0_m, -90_deg},
                  frc::Pose2d{0_m, 0_m, 135_deg},
                  frc::Pose2d{-3_m, 0_m, -90_deg},
                  frc::Pose2d{0_m, 0_m, 45_deg}},
      frc::TrajectoryConfig(5.0_mps, 2.0_mps_sq));

  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0.0, 1.0);

  units::second_t dt = 0.02_s;
  units::second_t t = 0_s;

  units::second_t kVisionUpdateRate = 0.1_s;
  frc::Pose2d lastVisionPose;
  units::second_t lastVisionUpdateTime{-std::numeric_limits<double>::max()};

  std::vector<frc::Pose2d> visionPoses;

  double maxError = -std::numeric_limits<double>::max();
  double errorSum = 0;

  while (t < trajectory.TotalTime()) {
    frc::Trajectory::State groundTruthState = trajectory.Sample(t);

    if (lastVisionUpdateTime + kVisionUpdateRate < t) {
      if (lastVisionPose != frc::Pose2d{}) {
        estimator.AddVisionMeasurement(lastVisionPose, lastVisionUpdateTime);
      }
      lastVisionPose =
          groundTruthState.pose +
          frc::Transform2d{
              frc::Translation2d{distribution(generator) * 0.1_m,
                                 distribution(generator) * 0.1_m},
              frc::Rotation2d{distribution(generator) * 0.1 * 1_rad}};
      visionPoses.push_back(lastVisionPose);
      lastVisionUpdateTime = t;
    }

    auto wheelSpeeds = kinematics.ToWheelSpeeds(
        {groundTruthState.velocity * groundTruthState.pose.Rotation().Cos(),
         groundTruthState.velocity * groundTruthState.pose.Rotation().Sin(),
         0_rad_per_s});

    wheelPositions.frontLeft += wheelSpeeds.frontLeft * dt;
    wheelPositions.frontRight += wheelSpeeds.frontRight * dt;
    wheelPositions.rearLeft += wheelSpeeds.rearLeft * dt;
    wheelPositions.rearRight += wheelSpeeds.rearRight * dt;

    auto xhat = estimator.UpdateWithTime(
        t, frc::Rotation2d{distribution(generator) * 0.05_rad}, wheelSpeeds,
        wheelPositions);
    double error = groundTruthState.pose.Translation()
                       .Distance(xhat.Translation())
                       .value();

    if (error > maxError) {
      maxError = error;
    }
    errorSum += error;

    t += dt;
  }

  EXPECT_LT(errorSum / (trajectory.TotalTime().value() / dt.value()), 0.05);
  EXPECT_LT(maxError, 0.125);
}
