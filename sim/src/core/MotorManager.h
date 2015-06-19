/*
 *  Copyright 2011, 2012, DFKI GmbH Robotics Innovation Center
 *
 *  This file is part of the MARS simulation framework.
 *
 *  MARS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3
 *  of the License, or (at your option) any later version.
 *
 *  MARS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with MARS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * \file MotorManager.h
 * \author  Vladimir Komsiyski
 * \brief "MotorManager" implements the MotorManagerInterface.
 * It is manages all motors and all motor
 * operations that are used for the communication between the simulation
 * modules.
 *
 * \version 1.3
 * Moved from the Simulator class
 * \date 07.07.2011
 */

#ifndef MOTOR_MANAGER_H
#define MOTOR_MANAGER_H

#ifdef _PRINT_HEADER_
  #warning "MotorManager.h"
#endif

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/utils/Mutex.h>

namespace mars {
  namespace sim {

    class SimMotor;

    /**
     * \brief "MotorManager" imlements the interfaces for all motor
     * operations that are used for the communication between the simulation
     * modules. Inherits from MotorManagerInterface.
     *
     * \warning It is very important to assure the serialization between the threads to
     * have the desired results. Currently the verified use of the functions
     * is only guaranteed by calling it within the main thread (update
     * callback from \c gui_thread).
     */
    class MotorManager : public interfaces::MotorManagerInterface {
    public:

      MotorManager(interfaces::ControlCenter *c);
      virtual ~MotorManager(){}

      // functional methods
      virtual unsigned long addMotor(interfaces::MotorData *motordata,
        bool reload = false, const std::string type = "BaseMotor");
      virtual void clearMotors(bool clear_all = false);
      virtual void deactivateMotor(unsigned long id);
      virtual void editMotor(const interfaces::MotorData &motordata);
      virtual void moveMotor(unsigned long index, interfaces::sReal value);
      virtual void reloadMotors(void);
      virtual void removeMotor(unsigned long index);
      virtual void removeJointFromMotors(unsigned long joint_index);
      virtual void updateMotors(interfaces::sReal calc_ms);

      // getters
      virtual unsigned long getID(const std::string& motor_name) const;
      virtual int getMotorCount() const;
      virtual const interfaces::MotorData getMotorData(unsigned long index) const;
      virtual interfaces::sReal getMotorEffort(unsigned long motorId) const;
      virtual interfaces::sReal getMotorPosition(unsigned long motorId) const;
      virtual SimMotor* getSimMotor(unsigned long id) const;
      virtual SimMotor* getSimMotorByName(const std::string &name) const;
      virtual void retrieveMotorList(std::vector<interfaces::core_objects_exchange> *motorList) const;

      // setters
      virtual void setMotorMaxSpeed(unsigned long id, interfaces::sReal maxSpeed);
      virtual void setMotorValueDesiredVelocity(unsigned long id, interfaces::sReal velocity);
      virtual void setMotorP(unsigned long id, interfaces::sReal value);
      virtual void setMotorI(unsigned long id, interfaces::sReal value);
      virtual void setMotorD(unsigned long id, interfaces::sReal value);
      virtual void setMotorValue(unsigned long id, interfaces::sReal value);

      // methods inherited from DataBrokerInterface
      virtual void getDataBrokerNames(unsigned long jointId,
                                      std::string *groupName,
                                      std::string *dataName) const;

      // deprecated
      virtual void clearAllMotors(bool clear_all = false) __attribute__ ((deprecated("use clearMotors")));
      virtual interfaces::sReal getActualPosition(unsigned long motorId) const __attribute__ ((deprecated("use getMotorPosition")));
      virtual const interfaces::MotorData getFullMotor(unsigned long index) const __attribute__ ((deprecated("use getMotorData")));
      virtual void getListMotors(std::vector<interfaces::core_objects_exchange> *motorList) const __attribute__ ((deprecated("use retrieveMotorList")));
      virtual interfaces::sReal getTorque(unsigned long motorId) const __attribute__ ((deprecated("use getMotorEffort")));
      virtual void setMaxTorque(unsigned long id, interfaces::sReal maxTorque) __attribute__ ((deprecated("use setMotorMaxEffort")));
      virtual void setMaxSpeed(unsigned long id, interfaces::sReal maxSpeed) __attribute__ ((deprecated("use setMotorMaxSpeed")));

    private:
      //! the id of the next motor that is added to the simulation
      unsigned long next_motor_id;

      //! a container for all motors currently present in the simulation
      std::map<unsigned long, SimMotor*> motors;

      //! a containter for all motors that are reloaded after a reset of the simulation
      std::list<interfaces::MotorData> reloadList;
      interfaces::ControlCenter *control;

      //! a mutex for the motor containers
      mutable utils::Mutex iMutex;

    }; // class MotorManager

  } // end of namespace sim
} // end of namespace mars

#endif  // MOTOR_MANAGER_H
