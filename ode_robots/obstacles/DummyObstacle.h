/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *    guettler@informatik.uni-leipzig.de                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *                                                                         *
 *  DESCRIPTION                                                            *
 *  
 *                                                                         *
 *                                                                         *
 *  $Log$
 *  Revision 1.2  2009-11-12 16:06:44  guettler
 *  header guard corrected
 *
 *  Revision 1.1  2009/11/12 16:05:11  guettler
 *  new DummyObstacle which uses a DummyPrimitive
 *										   *
 *                                                                         *
 **************************************************************************/
#ifndef __DUMMYOBSTACLE_H_
#define __DUMMYOBSTACLE_H_

#include "abstractobstacle.h"

namespace lpzrobots {

  /**
   * DummyObstacle which holds a DummyPrimitive (so getMainPrimitive() works).
   */
  class DummyObstacle : public lpzrobots::AbstractObstacle {
    public:
      DummyObstacle(const OdeHandle& odeHandle, const OsgHandle& osgHandle);
      virtual ~DummyObstacle();

      /**
       * sets position of the obstacle and creates/recreates obstacle if necessary
       */
      virtual void setPose(const osg::Matrix& pose);

      /// return the "main" primitive of the obtactle. The meaning of "main" is arbitrary
      virtual Primitive* getMainPrimitive() const;

    protected:
      /// overload this function to create the obstactle. All primitives should go into the list "obst"
      virtual void create();
  };

}

#endif /* __DUMMYOBSTACLE_H_ */