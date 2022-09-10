#include <gtest/gtest.h>
#include "fortress/constants/GConstants.h"
#include "fortress/streams/GFileStream.h"
#include "fortress/system/path/GFile.h"
#include "fortress/time/GStopwatchTimer.h"
#include "heave/ode/GOde.h"
#include "heave/ode/GOdeSolver.h"
#include <iostream>
#include <sstream> 

// Tests
using namespace rev;


/// @brief SpringOde class
class SpringOde: public Ode<2, double> {
public:

    /// @param mass mass of the spring
    /// @param mu damping coefficient
    /// @param k spring stiffness
    /// @param x0 spring initial position
    SpringOde(double mass, double mu, double k, double x0) {
        // Initialize fields declared in the class.
        m_mass = mass;
        m_mu = mu;
        m_k = k;
        m_x0 = x0;

        // Set the initial conditions of the dependent
        // variables.
        // q[0] = vx
        // q[1] = x;
        setDependentVariable(0, 0.0);
        setDependentVariable(1, x0);
        setIndependentVariable(0.0);
    }
    double getVx() {
        return getDependentVariable(0);
    }
    double getX() const {
        return getDependentVariable(1);
    }
    double getTime() const {
        return getIndependentVariable();
    }

    /// @brief This method updates the velocity and position of the
    // spring using a 4th order Runge-Kutta solver to integrate the equations of motion.
    void updatePositionAndVelocityRk4(double dt) {
        OdeSolver::Rk4(
            m_dependentVariables,
            m_independentVariable,
            m_dependentVariables,
            m_independentVariable,
            dt,
            [this](
                double& dvx,
                double s,
                const ArrayType& newQ)
            {
                // Compute right-hand side values.
                dvx = -(m_mu * newQ[0] + m_k * newQ[1]) / m_mass;
            },
            [this](
                double& dx,
                double s,
                const ArrayType& newQ)
            {
                // Compute right-hand side values.
                dx = (newQ[0]);
            }
            );
    }
    void updatePositionAndVelocityRk45(double nextStepTime, double& outNextTimeStep, bool& outSuccess, double tolerance) {
        OdeSolver::Rk45(
            m_dependentVariables,
            m_independentVariable,
            outNextTimeStep,
            outSuccess,
            m_dependentVariables,
            m_independentVariable,
            tolerance, 
            nextStepTime,
            [this](
                double& dvx,
                double s,
                const ArrayType& newQ)
            {
                // Compute right-hand side values.
                dvx = -(m_mu * newQ[0] + m_k * newQ[1]) / m_mass;
            },
            [this](
                double& dx,
                double s,
                const ArrayType& newQ)
            {
                // Compute right-hand side values.
                dx = (newQ[0]);
            });
    }

    double m_mass;
    double m_mu;
    double m_k;
    double m_x0;
};


class Rk4Spring
{
public:

    /// @brief Returns positions of the spring over seven seconds
    static std::vector<std::pair<double, double>> MainRk4(bool writeFile = true) {
        std::stringstream sstream;
        GString myString;

        // Create a SpringOde object that represents
        // a 1.0 kg spring with a spring constant of
        // 20 N/m and a damping coefficient of 1.5 N-s/m
        double mass = 1.0;
        double mu = 1.5;
        double k = 20.0;

        double x0 = -0.2;
        SpringOde ode(mass, mu, k, x0);

        // Solve the ODE over a range of 7 seconds
        // using a 0.1 second time increment.
        double dt = 0.1;
        if (writeFile) {
            sstream << ("t, x, v\n");
            sstream << ode.getTime() << "," << ode.getX() << "," << ode.getVx() << '\n';
        }
        std::vector<std::pair<double, double>> posVel;
        posVel.emplace_back(ode.getX(), ode.getVx());
        while (ode.getTime() <= 7.0) {
            ode.updatePositionAndVelocityRk4(dt);
            if (writeFile) {
                sstream << ode.getTime() << "," << ode.getX() << "," << ode.getVx() << '\n';
            }
            posVel.emplace_back(ode.getX(), ode.getVx());
        }

        if (writeFile) {
            // Write to a file for visualization
            myString = sstream.str();
            FileStream fileStream(_MY_TEST_DIR + GString("/data/spring.csv"));
            fileStream.open(FileAccessMode::kWrite | FileAccessMode::kBinary | FileAccessMode::kTruncate);
            fileStream.write(myString.c_str(), myString.length(), false);
            fileStream.close();
        }
        return posVel;
    }


    static std::vector<std::pair<double, double>> MainRk45(bool writeFile = true) {
        GString myString;
        std::stringstream sstream;

        // Create a SpringOde object that represents
        // a 1.0 kg spring with a spring constant of
        // 20 N/m and a damping coefficient of 1.5 N-s/m
        double mass = 1.0;
        double mu = 1.5;
        double k = 20.0;

        double x0 = -0.2;
        SpringOde ode(mass, mu, k, x0);

        // Solve the ODE over a range of 7 seconds
        /// ~using a tolerance~
        double tolerance = 1e-4; // Relative error tolerance

        if (writeFile) {
            sstream << ("t, x, v\n");
            sstream << ode.getTime() << "," << ode.getX() << "," << ode.getVx() << '\n';
        }
        std::vector<std::pair<double, double>> posVel;
        posVel.emplace_back(ode.getX(), ode.getVx());
        double nextTimeStep = 0.2;
        bool succeeded;
        while (ode.getTime() <= 7.0) {
            ode.updatePositionAndVelocityRk45(nextTimeStep, nextTimeStep, succeeded, tolerance);
            if (succeeded) {
                if (writeFile) {
                    sstream << ode.getTime() << "," << ode.getX() << "," << ode.getVx() << '\n';
                }
                posVel.emplace_back(ode.getX(), ode.getVx());
            }
        }

        if (writeFile) {
            // Write to a file for visualization
            myString = sstream.str();
            FileStream fileStream(_MY_TEST_DIR + GString("/data/spring_rk45.csv"));
            fileStream.open(FileAccessMode::kWrite | FileAccessMode::kBinary | FileAccessMode::kTruncate);
            fileStream.write(myString.c_str(), myString.length(), false);
            fileStream.close();
        }
        return posVel;
    }
};

TEST(OdeTest, MassSpringSystem)
{
    StopwatchTimer timer;

    std::vector<std::pair<double, double>> posVel = Rk4Spring::MainRk4();

    // Compare with expected results
    EXPECT_EQ(posVel[0].first, -0.2);
    EXPECT_EQ(posVel[0].second, 0);
    EXPECT_NEAR(posVel[14].first, -0.068271, 5e-6);
    EXPECT_NEAR(posVel[14].second, -0.035786, 5e-6);
    EXPECT_NEAR(posVel[35].first, 0.0132627, 5e-6);
    EXPECT_NEAR(posVel[35].second, 0.018342, 5e-6);

    std::vector<std::pair<double, double>> posVelRk45 = Rk4Spring::MainRk45();

    GString command = GString(
        "C:/Users/dante/Documents/Projects/grand-blue-engine/tools/kindler/venvs/envs/deployment_38_x64/Scripts/python.exe")
        + " " _MY_TEST_DIR + GString("/data/plot_spring.py");
    system(command.c_str());

}


TEST(DISABLED_OdeTest, MassSpringSystemPerformance)
{
    StopwatchTimer timer;

    std::vector<std::pair<double, double>> posVel;
    timer.start();
    constexpr int count = 1000000;
    for (int i = 0; i < count; i++) {
        posVel = Rk4Spring::MainRk4(false);
    }
    double time1Sec = timer.getElapsed<double>();

    timer.restart();
    for (int i = 0; i < count; i++) {
        posVel = Rk4Spring::MainRk45(false);
    }
    double time4Sec = timer.getElapsed<double>();

}
