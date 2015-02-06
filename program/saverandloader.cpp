//Copyright 2015 Ryan Wick

//This file is part of Grovolve.

//Grovolve is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//Grovolve is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with Grovolve.  If not, see <http://www.gnu.org/licenses/>.


#include "saverandloader.h"
#include "environment.h"
#include "../settings/environmentsettings.h"
#include "../settings/simulationsettings.h"
#include "stats.h"
#include "../plant/genome.h"
#include "../plant/plantpart.h"
#include "../plant/organism.h"
#include "../plant/seed.h"
#include <fstream>
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/filter/gzip.hpp"


void SaverAndLoader::saveSimulation()
{
    std::ofstream ofs(m_fullFileName.toLocal8Bit().data(), std::ios_base::out | std::ios::binary);
    boost::iostreams::filtering_ostream out;
    out.push(boost::iostreams::gzip_compressor(1)); //1 is the compression level - I chose a low one for speed.
    out.push(ofs);
    boost::archive::text_oarchive ar(out);

    Stats * tempStats;
    long long elapsedTime = 0;
    if (!m_history)
    {
        elapsedTime = m_environment->getElapsedTime();
        m_environment->setElapsedTime(0);
        tempStats = new Stats();
        std::swap(m_stats, tempStats);
    }

    ar << *m_environment << *m_environmentSettings << *m_simulationSettings << *m_stats;

    if (!m_history)
    {
        m_environment->setElapsedTime(elapsedTime);
        std::swap(m_stats, tempStats);
        delete tempStats;
    }

    emit finishedSaving();
}

void SaverAndLoader::loadSimulation()
{
    std::ifstream ifs(m_fullFileName.toLocal8Bit().data(), std::ios_base::in | std::ios_base::binary);
    boost::iostreams::filtering_istream in;
    in.push(boost::iostreams::gzip_decompressor());
    in.push(ifs);
    boost::archive::text_iarchive ar(in);

    ar >> *m_environment >> *m_environmentSettings >> *m_simulationSettings >> *m_stats;

    emit finishedLoading();
}


