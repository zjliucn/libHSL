/******************************************************************************
 * $Id$
 *
 * Project:  libLAS - http://hsl.org - A BSD library for LAS format data.
 * Purpose:  LAS index class 
 * Author:   Gary Huber, gary@garyhuberart.com
 *
 ******************************************************************************
 * Copyright (c) 2010, Gary Huber, gary@garyhuberart.com
 *
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following 
 * conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided 
 *       with the distribution.
 *     * Neither the name of the Martin Isenburg or Iowa Department 
 *       of Natural Resources nor the names of its contributors may be 
 *       used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 ****************************************************************************/

#include "Index.h"
#include <string>
#include "Writer.h"
#include "index/IndexOutput.h"
#include "index/IndexCell.h"


namespace hsl
{

Index::Index()
{
	SetValues();
	m_indexBuilt = false;
} // Index::Index

Index::Index(IndexData const& ParamSrc)
{
	SetValues();
	Prep(ParamSrc);
} // Index::Index

bool Index::Prep(IndexData const& ParamSrc)
{

	m_reader = ParamSrc.m_reader;
	m_idxreader = ParamSrc.m_idxreader;
 	m_readerCreated = false;
	if (! m_reader)
	{
		try {
			// fails if input stream is invalid
			m_reader = new hsl::Reader(std::string(ParamSrc.m_ifs));
 			m_readerCreated = true;
 		} // try
 		catch (std::runtime_error const&) {
 			return (InputFileError("Index::Prep"));
 		} // catch
	} // if
	m_ofs = ParamSrc.m_ofs;
    m_debugOutputLevel = ParamSrc.m_debugOutputLevel;
	m_tempFileName = ParamSrc.m_tempFileName ? ParamSrc.m_tempFileName: "";
	m_indexAuthor = ParamSrc.m_indexAuthor ? ParamSrc.m_indexAuthor: "";
	m_indexComment = ParamSrc.m_indexComment ? ParamSrc.m_indexComment: "";
	m_indexDate = ParamSrc.m_indexDate ? ParamSrc.m_indexDate: "";
	m_cellSizeZ = ParamSrc.m_cellSizeZ;
	m_debugger = ParamSrc.m_debugger ? ParamSrc.m_debugger: stderr;
	m_readOnly = ParamSrc.m_readOnly;
	m_writestandaloneindex = ParamSrc.m_writestandaloneindex;
	m_forceNewIndex = ParamSrc.m_forceNewIndex;
	if (ParamSrc.m_maxMemoryUsage > 0)
		m_maxMemoryUsage = ParamSrc.m_maxMemoryUsage;
	else
		m_maxMemoryUsage = LIBHSL_INDEX_MAXMEMDEFAULT;
	if (m_maxMemoryUsage < LIBHSL_INDEX_MINMEMDEFAULT)
		m_maxMemoryUsage = LIBHSL_INDEX_MINMEMDEFAULT;
	m_indexBuilt = IndexInit();
	return (m_indexBuilt);
	
} // Index::Prep

void Index::SetValues(void)
{
	m_bounds.dimension(3);
    m_reader = 0;
	m_idxreader = 0;
    m_ofs = 0;
  	m_readerCreated = false;
	m_tempFile = 0;
	m_outputFile = 0;
    m_debugOutputLevel = 0;
    m_tempFileName = "";
	m_indexAuthor = "";
	m_indexComment = "";
	m_indexDate = "";
	m_versionMajor = LIBHSL_INDEX_VERSIONMAJOR;
	m_versionMinor = LIBHSL_INDEX_VERSIONMINOR;
	m_cellSizeZ = 0.0;
	m_debugger = stderr;
	m_readOnly = false;
	m_forceNewIndex = false;
	m_DataVLR_ID = 43;
	m_maxMemoryUsage = LIBHSL_INDEX_MAXMEMDEFAULT;
    m_rangeX = m_rangeY = m_rangeZ = m_cellSizeZ = m_cellSizeX = m_cellSizeY = 
		m_pointRecordsCount = m_maxMemoryUsage = m_cellsX = m_cellsY = m_cellsZ = m_totalCells = 0;
    m_tempFileWrittenBytes = 0;
	m_indexBuilt = m_tempFileStarted = m_readerCreated = false;
} // Index::SetValues

Index::~Index(void)
{
	if (m_readerCreated)
		delete m_reader;
} // Index::~Index

bool Index::IndexInit(void)
{
	bool Success = false;
	bool IndexFound = false;

	if (m_idxreader || m_reader)
	{
		if (m_idxreader)
		{
			m_idxheader = m_idxreader->getHeader();
		} // if
		else if (m_reader)
		{
			m_idxheader = m_reader->getHeader();
		} // else
		if (m_reader)
		{
			m_pointheader = m_reader->getHeader();
		} // else

		if (IndexFound)
		{
			if (m_forceNewIndex)
			{
				ClearOldIndex();
				IndexFound = false;
				if (m_debugOutputLevel > 1)
					fprintf(m_debugger, "Old index removed.\n");
			} // if
			else if (! Validate())
			{
				IndexFound = false;
				if (m_debugOutputLevel > 1)
					fprintf(m_debugger, "Existing index out of date.\n");
			} // else if failed index validation test
			else
				return (true);
		} // if
		if (! IndexFound)
		{
			if (! m_readOnly)
			{
				Success = BuildIndex();
			} // if
			else if (m_debugOutputLevel > 1)
				fprintf(m_debugger, "Index not found nor created per user instructions.\n");
		} // if
		return Success;
	} // m_reader
	return (InitError("Index::IndexInit"));	
} // Index::IndexInit

void Index::ClearOldIndex(void)
{
} // Index::ClearOldIndex

bool Index::Validate(void)
{

	// compare the index-stored values to the ones in the las file header
	// Index needs to be current in order to be valid.
	// The validity test cannot determine if points have been moved but if the number of points or the 
	// bounds of the points has changed since the index was built then the index is deemed invalid and 
	// not to be used for filtering.
    Bounds<double> HeaderBounds(m_pointheader.getMinX(), m_pointheader.getMinY(), m_pointheader.getMinZ(), m_pointheader.getMaxX(), m_pointheader.getMaxY(), m_pointheader.getMaxZ());
	if (m_bounds == HeaderBounds)
	{
		if (m_pointheader.getPointRecordsCount() == GetPointRecordsCount())
			return (true);
	} // if
	return (false);
	
} // Index::Validate

uint32_t Index::GetDefaultReserve(void)
{
	return (GetPointRecordsCount() < LIBHSL_INDEX_RESERVEFILTERDEFAULT ? GetPointRecordsCount(): LIBHSL_INDEX_RESERVEFILTERDEFAULT);
} // Index::GetDefaultReserve

const std::vector<uint32_t>& Index::Filter(IndexData & ParamSrc)
{

	try {
		// if there is already a list, get rid of it
		m_filterResult.resize(0);
		// are we asked to advance beyond the number of points in the file? that would be a mistake and waste of time.
		if (ParamSrc.m_iterator && 
			(ParamSrc.m_iterator->m_advance + ParamSrc.m_iterator->m_totalPointsScanned > GetPointRecordsCount()))
			return (m_filterResult);
			
		m_filterResult.reserve(ParamSrc.m_iterator ? ParamSrc.m_iterator->m_chunkSize: GetDefaultReserve());
	} // try
	catch (std::bad_alloc const&) {
		m_filterResult.resize(0);
	} // catch
	return (m_filterResult);

} // Index::Filter

IndexIterator* Index::Filter(IndexData const& ParamSrc, uint32_t ChunkSize)
{
	IndexIterator* NewIter = NULL;

	try {
		NewIter = new IndexIterator(this, ParamSrc, ChunkSize);
	} // try
	catch (std::bad_alloc const&) {
		return (NULL);
	} // catch

	return (NewIter);

} // Index::Filter

IndexIterator* Index::Filter(double LowFilterX, double HighFilterX, double LowFilterY, double HighFilterY, 
	double LowFilterZ, double HighFilterZ, uint32_t ChunkSize)
{
	IndexIterator* NewIter = NULL;

	try {
		NewIter = new IndexIterator(this, LowFilterX, HighFilterX, LowFilterY, HighFilterY, 
			LowFilterZ, HighFilterZ, ChunkSize);
	} // try
	catch (std::bad_alloc const&) {
		return (NULL);
	} // catch

	return (NewIter);

} // Index::Filter

IndexIterator* Index::Filter(Bounds<double> const& BoundsSrc, uint32_t ChunkSize)
{
	IndexIterator* NewIter = NULL;

	try {
		NewIter = new IndexIterator(this, BoundsSrc, ChunkSize);
	} // try
	catch (std::bad_alloc const&) {
		return (NULL);
	} // catch

	return (NewIter);

} // Index::Filter

void Index::SetCellFilterBounds(IndexData & ParamSrc)
{
	double LowXCell, HighXCell, LowYCell, HighYCell, LowZCell, HighZCell,
		filterMinXCell, filterMaxXCell, filterMinYCell, filterMaxYCell, filterMinZCell, filterMaxZCell;
	// convert filter bounds into cell numbers
	// X and Y range can not be 0 or the index would not have been built
	filterMinXCell = m_cellsX * (ParamSrc.GetMinFilterX() - GetMinX()) / GetRangeX();
	filterMaxXCell = m_cellsX * (ParamSrc.GetMaxFilterX() - GetMinX()) / GetRangeX();
	filterMinYCell = m_cellsY * (ParamSrc.GetMinFilterY() - GetMinY()) / GetRangeY();
	filterMaxYCell = m_cellsY * (ParamSrc.GetMaxFilterY() - GetMinY()) / GetRangeY();
	// Z range however can be 0
	if (GetRangeZ() > 0.0 && ! detail::compare_distance(GetRangeZ(), 0.0))
	{
		filterMinZCell = m_cellsZ * (ParamSrc.GetMinFilterZ() - GetMinZ()) / GetRangeZ();
		filterMaxZCell = m_cellsZ * (ParamSrc.GetMaxFilterZ() - GetMinZ()) / GetRangeZ();
	} // if
	else
	{
		filterMinZCell = filterMaxZCell = 0;
	} // else
	LowXCell = ceil(filterMinXCell);
	HighXCell = floor(filterMaxXCell) - 1.0;
	LowYCell = ceil(filterMinYCell);
	HighYCell = floor(filterMaxYCell) - 1.0;
	LowZCell = ceil(filterMinZCell);
	HighZCell = floor(filterMaxZCell) - 1.0;
	ParamSrc.m_LowXCellCompletelyIn = (int32_t)LowXCell;
	ParamSrc.m_HighXCellCompletelyIn = (int32_t)HighXCell;
	ParamSrc.m_LowYCellCompletelyIn = (int32_t)LowYCell;
	ParamSrc.m_HighYCellCompletelyIn = (int32_t)HighYCell;
	ParamSrc.m_LowZCellCompletelyIn = (int32_t)LowZCell;
	ParamSrc.m_HighZCellCompletelyIn = (int32_t)HighZCell;

    LowXCell = floor(filterMinXCell);
    HighXCell = ceil(filterMaxXCell) - 1.0;
    LowYCell = floor(filterMinYCell);
    HighYCell = ceil(filterMaxYCell) - 1.0;
    LowZCell = floor(filterMinZCell);
    HighZCell = ceil(filterMaxZCell) - 1.0;
    ParamSrc.m_LowXBorderCell = (int32_t)LowXCell;
    ParamSrc.m_HighXBorderCell = (int32_t)HighXCell;
    ParamSrc.m_LowYBorderCell = (int32_t)LowYCell;
    ParamSrc.m_HighYBorderCell = (int32_t)HighYCell;
    ParamSrc.m_LowZBorderCell = (int32_t)LowZCell;
    ParamSrc.m_HighZBorderCell = (int32_t)HighZCell;
    
    ParamSrc.m_LowXBorderPartCell = filterMinXCell - LowXCell;
    ParamSrc.m_HighXBorderPartCell = filterMaxXCell - HighXCell;
    ParamSrc.m_LowYBorderPartCell = filterMinYCell - LowYCell;
    ParamSrc.m_HighYBorderPartCell = filterMaxYCell - HighYCell;
	
} // Index::SetCellFilterBounds

bool Index::FilterPointSeries(uint32_t & PointID, uint32_t & PointsScanned, 
	uint32_t const PointsToIgnore, uint32_t const x, uint32_t const y, uint32_t const z, 
	hsl::detail::ConsecPtAccumulator const ConsecutivePts, IndexIterator *Iterator, 
	IndexData const& ParamSrc)
{
	bool LastPtRead = 0;
	uint32_t LastPointID = static_cast<uint32_t>(~0);
	
	try {	
		for (uint32_t PtCt = 0; PtCt < ConsecutivePts; ++PointID, ++PtCt)
		{
			++PointsScanned;
			if (Iterator)
				++Iterator->m_ptsScannedCurCell;
			if (PointsScanned > PointsToIgnore)
			{
				if (FilterOnePoint(x, y, z, PointID, LastPointID, LastPtRead, ParamSrc))
				{
					bool SkipIt = false;
					if (Iterator)
					{
						++Iterator->m_conformingPtsFound;
						if (Iterator->m_advance)
						{
							--Iterator->m_advance;
							if (Iterator->m_advance)
								SkipIt = true;
						} // if
					} // if
					if (! SkipIt)
					{
						m_filterResult.push_back(PointID);
						if (Iterator && (m_filterResult.size() >= Iterator->m_chunkSize))
							break;
					} // if
				} // if
			} // if
			LastPointID = PointID;
		} // for
		return (true);
	} // try
	catch (std::bad_alloc const&) {
		return (false);
	} // catch

} // Index::FilterPointSeries

bool Index::CellInteresting(int32_t XCellID, int32_t YCellID, IndexData const& ParamSrc)
{

	if (ParamSrc.m_noFilterX || (XCellID >= ParamSrc.m_LowXBorderCell && XCellID <= ParamSrc.m_HighXBorderCell))
	{
		if (ParamSrc.m_noFilterY || (YCellID >= ParamSrc.m_LowYBorderCell && YCellID <= ParamSrc.m_HighYBorderCell))
			return true;
	} // if
	return false;

} // Index::CellInteresting

bool Index::ZCellInteresting(int32_t ZCellID, IndexData const& ParamSrc)
{

	if (ParamSrc.m_noFilterZ || (ZCellID >= ParamSrc.m_LowZBorderCell && ZCellID <= ParamSrc.m_HighZBorderCell))
	{
		return true;
	} // if
	return false;

} // Index::ZCellInteresting

bool Index::SubCellInteresting(int32_t SubCellID, int32_t XCellID, int32_t YCellID, IndexData const& ParamSrc)
{
	bool XGood = false, YGood = false;

	// only need test sub cell if border cell
	if (ParamSrc.m_noFilterX || (XCellID >= ParamSrc.m_LowXCellCompletelyIn && XCellID <= ParamSrc.m_HighXCellCompletelyIn))
	{
		XGood = true;
	} // if
	else
	{
		if (XCellID == ParamSrc.m_LowXBorderCell)
		{
			// left (low) border cell
			if (SubCellID == 0 || SubCellID == 2)
			{
				// low half of cell in X
				// only true if boundary of search area falls in the left (lower) half of the cell
				if (ParamSrc.m_LowXBorderPartCell <= .5)	// .5 exactly is part of low half of cell
					XGood = true;
			} // if
			else
			{
				// high half of cell in X
				// always true if boundary of search area falls anywhere in the cell
				XGood = true;
			} // else
		} // if
		else
		{
			// right (upper) border cell
			if (SubCellID == 0 || SubCellID == 2)
			{
				// low half of cell in X
				// always true if boundary of search area falls anywhere in the cell
				XGood = true;
			} // if
			else
			{
				// high half of cell in X
				// only true if boundary of search area falls in the right (upper) half of the cell
				if (ParamSrc.m_HighXBorderPartCell > .5)
					XGood = true;
			} // else
		} // else	
	} // else
	if (ParamSrc.m_noFilterY || (YCellID >= ParamSrc.m_LowYCellCompletelyIn && YCellID <= ParamSrc.m_HighYCellCompletelyIn))
	{
		YGood = true;
	} // if
	else
	{
		if (YCellID == ParamSrc.m_LowYBorderCell)
		{
			// bottom (low) border cell
			if (SubCellID == 0 || SubCellID == 1)
			{
				// low half of cell in Y
				// only true if boundary of search area falls in the bottom (lower) half of the cell
				if (ParamSrc.m_LowYBorderPartCell <= .5)	// .5 exactly is part of low half of cell
					YGood = true;
			} // if
			else
			{
				// high half of cell in Y
				// always true if boundary of search area falls anywhere in the cell
				YGood = true;
			} // else
		} // if
		else
		{
			// top (upper) border cell
			if (SubCellID == 0 || SubCellID == 1)
			{
				// low half of cell in Y
				// always true if boundary of search area falls anywhere in the cell
				YGood = true;
			} // if
			else
			{
				// high half of cell in Y
				// only true if boundary of search area falls in the top (upper) half of the cell
				if (ParamSrc.m_HighYBorderPartCell > .5)
					YGood = true;
			} // else
		} // else	
	} // else
	
	return (XGood && YGood);

} // Index::SubCellInteresting

bool Index::FilterOnePoint(int32_t x, int32_t y, int32_t z, int32_t PointID, int32_t LastPointID, bool &LastPtRead, 
	IndexData const& ParamSrc)
{
	bool XGood = false, YGood = false, ZGood = false, PtRead = false;
	double PtX, PtY = 0.0, PtZ = 0.0;
	
	// filtering turned on for X?
	if (ParamSrc.m_noFilterX)
		XGood = true;
	// container cell in the region of the filter that requires no individual point testing?
	else if (x >= ParamSrc.m_LowXCellCompletelyIn && x <= ParamSrc.m_HighXCellCompletelyIn)
		XGood = true;
	// container cell in the region of the filter that does require individual point testing?
	else if (x == ParamSrc.m_LowXBorderCell || x == ParamSrc.m_HighXBorderCell)
	{
		// save a file seek if it is the subsequent point from the last one read
		if (PointID == LastPointID + 1)
		{
			if (LastPtRead)	
			{
				PtRead = m_reader->readNextPoint();
			} // if
		} // if
		if (! PtRead)
		{
			// seek and read
			assert(static_cast<uint32_t>(PointID) < m_pointRecordsCount);
			PtRead = (m_reader->seek(PointID) && m_reader->readNextPoint());
		} // if
		if (PtRead)
		{
			Point const& TestPt = m_reader->getPoint();
			PtX = TestPt.getX();
			PtY = TestPt.getY();
			PtZ = TestPt.getZ();
			if (PtX >= ParamSrc.GetMinFilterX() && PtX <= ParamSrc.GetMaxFilterX())
				XGood = true;
		} // if
	} // else
	if (XGood)
	{
		// filtering turned on for Y?
		if (ParamSrc.m_noFilterY)
			YGood = true;
		// container cell in the region of the filter that requires no individual point testing?
		else if (y >= ParamSrc.m_LowYCellCompletelyIn && y <= ParamSrc.m_HighYCellCompletelyIn)
			YGood = true;
		// container cell in the region of the filter that does require individual point testing?
		else if (y == ParamSrc.m_LowYBorderCell || y == ParamSrc.m_HighYBorderCell)
		{
			if (PtRead)
			{
				// we already have the data for this point
				if (PtY >= ParamSrc.GetMinFilterY() && PtY <= ParamSrc.GetMaxFilterY())
					YGood = true;
			}
			else
			{
				// save a file seek if it is the subsequent point from the last one read
				if (PointID == LastPointID + 1)
				{
					if (LastPtRead)	
					{
						PtRead = m_reader->readNextPoint();
					} // if
				} // if
				if (! PtRead)
				{
					// seek and read
					assert(static_cast<uint32_t>(PointID) < m_pointRecordsCount);
					PtRead = (m_reader->seek(PointID) && m_reader->readNextPoint());
				} // if
				if (PtRead)
				{
					Point const& TestPt = m_reader->getPoint();
					PtY = TestPt.getY();
					PtZ = TestPt.getZ();
					if (PtY >= ParamSrc.GetMinFilterY() && PtY <= ParamSrc.GetMaxFilterY())
						YGood = true;
				} // if
			} // else
		} // else if
	} // if
	if (XGood && YGood)
	{
		// filtering turned on for Z?
		if (ParamSrc.m_noFilterZ)
			ZGood = true;
		// container cell in the region of the filter that requires no individual point testing?
		else if (z >= ParamSrc.m_LowZCellCompletelyIn && z <= ParamSrc.m_HighZCellCompletelyIn)
			ZGood = true;
		// container cell in the region of the filter that does require individual point testing?
		else if (z == ParamSrc.m_LowZBorderCell || z == ParamSrc.m_HighZBorderCell)
		{
			if (PtRead)
			{
				// we already have the data for this point
				if (PtZ >= ParamSrc.GetMinFilterZ() && PtZ <= ParamSrc.GetMaxFilterZ())
					ZGood = true;
			}
			else
			{
				// save a file seek if it is the subsequent point from the last one read
				if (PointID == LastPointID + 1)
				{
					if (LastPtRead)	
					{
						PtRead = m_reader->readNextPoint();
					} // if
				} // if
				if (! PtRead)
				{
					// seek and read
					assert(static_cast<uint32_t>(PointID) < m_pointRecordsCount);
					PtRead = (m_reader->seek(PointID) && m_reader->readNextPoint());
				} // if
				if (PtRead)
				{
					Point const& TestPt = m_reader->getPoint();
					PtZ = TestPt.getZ();
					if (PtZ >= ParamSrc.GetMinFilterZ() && PtZ <= ParamSrc.GetMaxFilterZ())
						ZGood = true;
				} // if
			} // else
		} // else if
	} // if
	LastPtRead = PtRead;
	return (XGood && YGood && ZGood);
	
} // Index::FilterOnePoint

bool Index::BuildIndex(void)
{
	// Build an array of two dimensions. Sort data points into
	uint32_t MaximumCells = LIBHSL_INDEX_MAXCELLS;
	m_versionMajor = LIBHSL_INDEX_VERSIONMAJOR;
	m_versionMinor = LIBHSL_INDEX_VERSIONMINOR;
	
	// reset to beginning of point data records in case points had been examined before index is built
	m_reader->seek(0);
	// need the header to get number of point records
    m_pointRecordsCount = m_pointheader.getPointRecordsCount();
    // get the bounds of the data and scale factors in case they are needed for point translation
    m_bounds = Bounds<double>(m_pointheader.getMinX(), m_pointheader.getMinY(), m_pointheader.getMinZ(), m_pointheader.getMaxX(), m_pointheader.getMaxY(), m_pointheader.getMaxZ());
    try {
		m_bounds.verify();
	} // try
	catch (std::runtime_error const&) {
		return (InputBoundsError("Index::BuildIndex"));
	} // catch
	CalcRangeX();
	CalcRangeY(); 
	CalcRangeZ();

	if (m_cellSizeZ > 0.0 && ! detail::compare_distance(m_cellSizeZ, 0.0))
		m_cellsZ = static_cast<uint32_t>(ceil(m_rangeZ / m_cellSizeZ));
	else
		m_cellsZ = 1;
		
	// under the conditions of one dimension (x or y) being 0 or negative in size, no index is possible
	if ((m_bounds.max)(0) <= (m_bounds.min)(0) || (m_bounds.max)(1) <= (m_bounds.min)(1))
	{
		return (PointBoundsError("Index::BuildIndex"));
	} // if
			
	// fix a cell size and number of cells in X and Y to begin the process of indexing
	double XRatio = m_rangeX >= m_rangeY ? 1.0: m_rangeX / m_rangeY;
	double YRatio = m_rangeY >= m_rangeX ? 1.0: m_rangeY / m_rangeX;
	
	m_totalCells = m_pointRecordsCount / LIBHSL_INDEX_OPTPTSPERCELL;
	m_totalCells = static_cast<uint32_t>(sqrt((double)m_totalCells));
	if (m_totalCells < 10)
		m_totalCells = 10;	// let's set a minimum number of cells to make the effort worthwhile
	m_cellsX = static_cast<uint32_t>(XRatio * m_totalCells);
	m_cellsY = static_cast<uint32_t>(YRatio * m_totalCells);
	if (m_cellsX < 1)
		m_cellsX = 1;
	if (m_cellsY < 1)
		m_cellsY = 1;
	m_totalCells = m_cellsX * m_cellsY;
	if (m_totalCells > MaximumCells)
	{
		double CellReductionRatio = (double)MaximumCells / (double)m_totalCells;
		CellReductionRatio = sqrt(CellReductionRatio);
		m_cellsX = static_cast<uint32_t>(m_cellsX * CellReductionRatio);
		m_cellsY = static_cast<uint32_t>(m_cellsY * CellReductionRatio);
		m_totalCells = m_cellsX * m_cellsY;
	} // if
	m_cellSizeX = m_rangeX / m_cellsX;
	m_cellSizeY = m_rangeY / m_cellsY;

	// print some statistics to the console
	if (m_debugOutputLevel > 1)
	{
		fprintf(m_debugger, "Points in file %d, Cell matrix x %d, y %d, z %d\n", m_pointRecordsCount, m_cellsX, m_cellsY,
			m_cellsZ);
		fprintf(m_debugger, "Point ranges x %.2f-%.2f, y %.2f-%.2f, z %.2f-%.2f, z range %.2f\n", (m_bounds.min)(0), (m_bounds.max)(0), (m_bounds.min)(1), (m_bounds.max)(1), 
			(m_bounds.min)(2), (m_bounds.max)(2), m_rangeZ);
	} // if
	
	// now we know how large our index array is going to be
	// we'll create a vector of that many entities
	
	try {
		// a one dimensional array to represent cell matrix
		IndexCellRow IndexCellsY(m_cellsY);
		// a two dimensional array
		IndexCellDataBlock IndexCellBlock(m_cellsX, IndexCellsY);
		hsl::detail::IndexOutput IndexOut(this);
		
		// for Z bounds debugging
		uint32_t ZRangeSum = 0;
		uint32_t PointSum = 0;
		// read each point in the file
		// figure out what cell in X and Y
		// test to see if it is the same as the last cell
		uint32_t LastCellX = static_cast<uint32_t>(~0), LastCellY = static_cast<uint32_t>(~0);
		hsl::detail::ElevRange ZRange;
		uint32_t PointID = 0;
		uint32_t LastPointID = 0;
		uint32_t PtsIndexed = 0;
		uint32_t PointsInMemory = 0, MaxPointsInMemory;
		MaxPointsInMemory = m_maxMemoryUsage / sizeof(hsl::detail::IndexCell);
		// ReadNextPoint() throws a std::out_of_range error when it hits end of range so don't 
		// get excited when you see it in the debug output
		while (m_reader->readNextPoint())
		{
			uint32_t CurCellX, CurCellY;
			// analyze the point to determine its cell ID
			Point const& CurPt = m_reader->getPoint();
			if (IdentifyCell(CurPt, CurCellX, CurCellY))
			{
				// if same cell as last point, attempt to increment the count of consecutive points for the cell
				// otherwise add a new point, first checking to see if the memory allocated to this process is full
				if (! (CurCellX == LastCellX && CurCellY == LastCellY &&
					IndexCellBlock[CurCellX][CurCellY].IncrementPointRecord(LastPointID)))
				{
					// if memory allocated to this process is full, write all the point data to a temp file
					if (m_tempFileName.size() && PointsInMemory >= MaxPointsInMemory)
					{
						if (! PurgePointsToTempFile(IndexCellBlock))
							return (FileError("Index::BuildIndex"));
						PointsInMemory = 0;
					} // if
					IndexCellBlock[CurCellX][CurCellY].AddPointRecord(PointID);
					LastPointID = PointID;
					LastCellX = CurCellX;
					LastCellY = CurCellY;
					++PointsInMemory;
				} // else
			// update Z cell bounds
			IndexCellBlock[CurCellX][CurCellY].UpdateZBounds(CurPt.getZ());
			} // if
		++PointID;
		} // while
		// write remaining points to temp file
		if (m_tempFileName.size())
		{
			if (! PurgePointsToTempFile(IndexCellBlock))
				return (FileError("Index::BuildIndex"));
		} // if using temp file

		// print some statistics to the console
		if (m_debugOutputLevel > 2)
		{
			if (! OutputCellStats(IndexCellBlock))
			{
				return (DebugOutputError("Index::BuildIndex"));
			} // if
		} // if

		// Here's where it gets fun
		// Read the binned data from the temp file, one cell at a time
		// Store the data in Variable records section of the LAS file
		// If a cell contains too many points, subdivide the cell and save sub-cells within the cell structure
		// If Z-binning is desired, define the bounds of each Z zone and subdivide sort each cell's points into Z bins
		// Save Z bins within the cell structure.
		
		if (IndexOut.InitiateOutput())
		{
			for (uint32_t x = 0; x < m_cellsX; ++x)
			{
				for (uint32_t y = 0; y < m_cellsY; ++y)
				{
					if (m_debugOutputLevel > 3)
						fprintf(m_debugger, "reloading %d %d\n", x, y);
					if (LoadCellFromTempFile(&IndexCellBlock[x][y], x, y))
					{
						ZRange = IndexCellBlock[x][y].GetZRange();
						// if Z-binning is specified, create Z sub-cells first
						// otherwise, subdivide the cell by quadrants if the number of points in the cell 
						// exceeds LIBHSL_INDEX_MAXPTSPERCELL
						if ((m_cellsZ > 1 && ZRange > m_cellSizeZ) || 
							(IndexCellBlock[x][y].GetNumPoints() > LIBHSL_INDEX_MAXPTSPERCELL))
						{
							// walk the points in this cell and divvy them up into Z - cells or quadtree cells
							// create an iterator for the map
							// walk the map entities
							hsl::detail::IndexCellData::iterator MapIt = IndexCellBlock[x][y].GetFirstRecord();
							for (; MapIt != IndexCellBlock[x][y].GetEnd(); ++MapIt)
							{
								// get the actual point from the las file
								assert(MapIt->first < m_pointRecordsCount);
								if (m_reader->seek(MapIt->first) && m_reader->readNextPoint())
								{
									uint32_t FirstPt = 0, LastCellZ = static_cast<uint32_t>(~0);
									uint32_t LastSubCell = static_cast<uint32_t>(~0);
									for (hsl::detail::ConsecPtAccumulator PtsTested = 0; PtsTested < MapIt->second; )
									{
										Point const& CurPt = m_reader->getPoint();
										// Z cell subdivision takes precedence over sub-cell quadrant subdivision
										if (m_cellsZ > 1 && ZRange > m_cellSizeZ)
										{
											// for the number of consecutive points, identify the Z cell
											uint32_t CurCellZ;
											if (IdentifyCellZ(CurPt, CurCellZ))
											{
												// add a record to the z cell chain or increment existing record
												if (! (CurCellZ == LastCellZ && IndexCellBlock[x][y].IncrementZCell(CurCellZ, FirstPt)))
												{
													FirstPt = MapIt->first + PtsTested;
													IndexCellBlock[x][y].AddZCell(CurCellZ, FirstPt);
													LastCellZ = CurCellZ;
												} // else
											} // if
										} // if
										else
										{
											uint32_t CurSubCell;
											// for the number of consecutive points, identify the sub cell in a 2x2 matrix
											// 0 is lower left, 1 is lower right, 2 is upper left, 3 is upper right
											if (IdentifySubCell(CurPt, x, y, CurSubCell))
											{
												// add a record to the sub cell chain or increment existing record
												if (! (CurSubCell == LastSubCell && IndexCellBlock[x][y].IncrementSubCell(CurSubCell, FirstPt)))
												{
													FirstPt = MapIt->first + PtsTested;
													IndexCellBlock[x][y].AddSubCell(CurSubCell, FirstPt);
													LastSubCell = CurSubCell;
												} // else
											} // if
										} // else
										++PtsTested;
										if (PtsTested < MapIt->second)
										{
											if (! m_reader->readNextPoint())
												return (FileError("Index::BuildIndex"));
										} // if
									} // for
								} // if
								else
									return (FileError("Index::BuildIndex"));
							} // for
							IndexCellBlock[x][y].RemoveMainRecords();
						} // if
						// sum the points for later debugging
						PtsIndexed += IndexCellBlock[x][y].GetNumPoints();
						// write the cell out to permanent file VLR
						if (! IndexOut.OutputCell(&IndexCellBlock[x][y], x, y))
							return (FileError("Index::BuildIndex"));

						// some statistical stuff for z bounds debugging
						ZRangeSum += ZRange;
						++PointSum;
						
						// purge the memory for this cell
						IndexCellBlock[x][y].RemoveAllRecords();
					} // if
					else
					{
						return (FileError("Index::BuildIndex"));
					}  // else
				} // for y
			} // for x
			// done with this baby
			CloseTempFile();
			if (! IndexOut.FinalizeOutput())
				return (FileError("Index::BuildIndex"));
			if (m_debugOutputLevel)
			{
				if (PtsIndexed < m_pointRecordsCount)
				{
				fprintf(m_debugger, "%d of %d points in las file were indexed.\n", PtsIndexed, m_pointRecordsCount);
				} // if
			} // if
			if (m_debugOutputLevel > 2 && PointSum)
			{
				ZRangeSum /= PointSum;
				fprintf(m_debugger, "Z range average per cell %d\n", ZRangeSum);
			} // if
			if (m_writestandaloneindex)
			{
				// save a standalone index file
				if (! SaveIndexInStandAloneFile())
					return false;
			} // if
			else
			{
				// resave the entire file with new index VLR's
				if (! SaveIndexInLASFile())
					return false;
			} // else
		} // if
	} // try
	catch (std::bad_alloc const&) {
		CloseTempFile();
		return (MemoryError("Index::BuildIndex"));
	} // catch
	
	return true;

} // Index::BuildIndex

bool Index::IdentifyCell(Point const& CurPt, uint32_t& CurCellX, uint32_t& CurCellY) const
{
	double OffsetX, OffsetY;

	OffsetX = (CurPt.getX() - (m_bounds.min)(0)) / m_rangeX;
	if (OffsetX >= 0 && OffsetX < 1.0)
		CurCellX = static_cast<uint32_t>(OffsetX * m_cellsX);
	else if (detail::compare_distance(OffsetX, 1.0))
		CurCellX = m_cellsX - 1;
	else
	{
		return (PointBoundsError("Index::IdentifyCell"));
	} // else
	
	OffsetY = (CurPt.getY() - (m_bounds.min)(1)) / m_rangeY;
	if (OffsetY >= 0 && OffsetY < 1.0)
		CurCellY = static_cast<uint32_t>(OffsetY * m_cellsY);
	else if (detail::compare_distance(OffsetY, 1.0))
		CurCellY = m_cellsY - 1;
	else
	{
		return (PointBoundsError("Index::IdentifyCell"));
	} // else
	
	return true;

} // Index::IdentifyCell

bool Index::IdentifyCellZ(Point const& CurPt, uint32_t& CurCellZ) const
{
	double OffsetZ;

	OffsetZ = (CurPt.getZ() - (m_bounds.min)(2)) / m_rangeZ;
	if (OffsetZ >= 0 && OffsetZ < 1.0)
		CurCellZ = static_cast<uint32_t>(OffsetZ * m_cellsZ);
	else if (detail::compare_distance(OffsetZ, 1.0))
		CurCellZ = m_cellsZ - 1;
	else
	{
		return (PointBoundsError("Index::IdentifyCellZ"));
	} // else

	return true;

} // Index::IdentifyCellZ

bool Index::IdentifySubCell(Point const& CurPt, uint32_t x, uint32_t y, uint32_t& CurSubCell) const
{
	double Offset, CellMinX, CellMinY;

	CellMinX = x * m_cellSizeX + (m_bounds.min)(0);
	CellMinY = y * m_cellSizeY + (m_bounds.min)(1);
	// find point position in X
	Offset = (CurPt.getX() - CellMinX) / m_cellSizeX;
	if (Offset > .5)	//upper half X
	{
		// find point position in Y
		Offset = (CurPt.getY() - CellMinY) / m_cellSizeY;
		if (Offset > .5)
			CurSubCell = 3;	// upper half of Y, NE
		else	// <= .5
			CurSubCell = 1;	// lower half of Y, SE
	} // if
	else	// lower half X
	{
		// <= .5
		// find point position in Y
		Offset = (CurPt.getY() - CellMinY) / m_cellSizeY;
		if (Offset > .5)
			CurSubCell = 2;	// upper half of Y, NW
		else	// <= .5
			CurSubCell = 0;	// lower half of Y, SW
	} // else

	return true;

} // Index::IdentifySubCell

bool Index::PurgePointsToTempFile(IndexCellDataBlock& CellBlock)
{
	if (m_tempFile || OpenTempFile())
	{
		hsl::detail::TempFileOffsetType EmptyOffset = 0;	// this might not be large enough
		
		if (! m_tempFileStarted)
		{
			// there is some setup of the temp file to be done first
			// write out a block of file offsets the size of the number of cells
			for (uint32_t i = 0; i < m_totalCells; ++i)
			{
				if (fwrite(&EmptyOffset, sizeof(hsl::detail::TempFileOffsetType), 1, m_tempFile) < 1)
				{
					return (FileError("Index::PurgePointsToTempFile"));
				} // if error
			} // for
			m_tempFileWrittenBytes = m_totalCells * sizeof(hsl::detail::TempFileOffsetType);
			m_tempFileStarted = true;
		} // if
		for (uint32_t x = 0; x < m_cellsX; ++x)
		{
			for (uint32_t y = 0; y < m_cellsY; ++y)
			{
				uint32_t RecordsToWrite = CellBlock[x][y].GetNumRecords();
				if (RecordsToWrite)
				{
					// write the current file location in the cell block header
					// if cell block header is 0 write the current file location in the file header
					// otherwise write the current file location at the file location specified in the 
					// cell block header
					hsl::detail::TempFileOffsetType LastWriteLocation = CellBlock[x][y].GetFileOffset();
					if (LastWriteLocation == 0)
						LastWriteLocation = (x * m_cellsY + y) * sizeof(hsl::detail::TempFileOffsetType);
#ifdef _MSC_VER
					_fseeki64(m_tempFile, LastWriteLocation, SEEK_SET);
#else
					fseek(m_tempFile, LastWriteLocation, SEEK_SET);
#endif
					if (fwrite(&m_tempFileWrittenBytes, sizeof(hsl::detail::TempFileOffsetType), 1, m_tempFile) < 1)
						return (FileError("Index::PurgePointsToTempFile"));
					CellBlock[x][y].SetFileOffset(m_tempFileWrittenBytes);

					// seek to end of file where next block of data will be written
#ifdef _MSC_VER
					_fseeki64(m_tempFile, 0, SEEK_END);
#else
					fseek(m_tempFile, 0, SEEK_END);
#endif

					// write a blank space for later placement of next file block for this cell
					if (fwrite(&EmptyOffset, sizeof(hsl::detail::TempFileOffsetType), 1, m_tempFile) < 1)
						return (FileError("Index::PurgePointsToTempFile"));
					m_tempFileWrittenBytes += sizeof(hsl::detail::TempFileOffsetType);
					// write the number of records stored in this section
					if (fwrite(&RecordsToWrite, sizeof(uint32_t), 1, m_tempFile) < 1)
						return (FileError("Index::PurgePointsToTempFile"));
					m_tempFileWrittenBytes += sizeof(uint32_t);

					hsl::detail::IndexCellData::iterator MapIt = CellBlock[x][y].GetFirstRecord();
					for (uint32_t RecordNum = 0; RecordNum < RecordsToWrite && MapIt != CellBlock[x][y].GetEnd(); ++RecordNum, ++MapIt)
					{
						// write the point ID
						uint32_t PointID = MapIt->first;
						// write the number of consecutive points
						hsl::detail::ConsecPtAccumulator ConsecutivePoints = MapIt->second;
						if (fwrite(&PointID, sizeof(uint32_t), 1, m_tempFile) < 1)
							return (FileError("Index::PurgePointsToTempFile"));
						if (fwrite(&ConsecutivePoints, sizeof(hsl::detail::ConsecPtAccumulator), 1, m_tempFile) < 1)
							return (FileError("Index::PurgePointsToTempFile"));
						m_tempFileWrittenBytes += sizeof(uint32_t);
						m_tempFileWrittenBytes += sizeof(hsl::detail::ConsecPtAccumulator);
					} // for
					// purge the records for this cell from active memory
					CellBlock[x][y].RemoveMainRecords();
				} // if
			} // for y
		} // for x
		// necessary for subsequent reads in case fseek isn't called first
		fflush(m_tempFile);
		return true;
	} // if file

	return (FileError("Index::PurgePointsToTempFile"));

} // Index::PurgePointsToTempFile

bool Index::LoadCellFromTempFile(hsl::detail::IndexCell *CellBlock, 
	uint32_t CurCellX, uint32_t CurCellY)
{

	uint32_t RecordsToRead, FormerNumPts, NewNumPts = 0;
	hsl::detail::TempFileOffsetType FileOffset;
	
	FormerNumPts = CellBlock->GetNumPoints();
	CellBlock->SetNumPoints(0);
	
	// load the cell as it was written
	// read the first offset for this cell

#ifdef _MSC_VER
	if (_fseeki64(m_tempFile, (CurCellX * m_cellsY + CurCellY) * sizeof (hsl::detail::TempFileOffsetType), SEEK_SET))
#else
	if (fseek(m_tempFile, (CurCellX * m_cellsY + CurCellY) * sizeof (hsl::detail::TempFileOffsetType), SEEK_SET))
#endif
		return (FileError("Index::LoadCellFromTempFile"));
	if (fread(&FileOffset, sizeof (hsl::detail::TempFileOffsetType), 1, m_tempFile) < 1)
		return (FileError("Index::LoadCellFromTempFile"));
	while (FileOffset > 0)
	{
		// jump to the first block for this cell, read the next offset

#ifdef _MSC_VER
		if (_fseeki64(m_tempFile, FileOffset, SEEK_SET))
#else
		if (fseek(m_tempFile, FileOffset, SEEK_SET))
#endif

			return (FileError("Index::LoadCellFromTempFile"));
		if (fread(&FileOffset, sizeof (hsl::detail::TempFileOffsetType), 1, m_tempFile) < 1)
			return (FileError("Index::LoadCellFromTempFile"));
		// read the data for the cell in this block
		// first is the number of items to read now
		if (fread(&RecordsToRead, sizeof (uint32_t), 1, m_tempFile) < 1)
			return (FileError("Index::LoadCellFromTempFile"));
		for (uint32_t RecordNum = 0; RecordNum < RecordsToRead; ++RecordNum)
		{
			uint32_t PointID;
			hsl::detail::ConsecPtAccumulator ConsecutivePoints;
			// read the point ID
			if (fread(&PointID, sizeof(uint32_t), 1, m_tempFile) < 1)
				return (FileError("Index::LoadCellFromTempFile"));
			// read the number of consecutive points
			if (fread(&ConsecutivePoints, sizeof(hsl::detail::ConsecPtAccumulator), 1, m_tempFile) < 1)
				return (FileError("Index::LoadCellFromTempFile"));
			CellBlock->AddPointRecord(PointID, ConsecutivePoints);
		} // for
	} // while
	// check to see that we got the number of points back that we started with
	NewNumPts = CellBlock->GetNumPoints();
	if (NewNumPts != FormerNumPts)
	{
		CloseTempFile();
		return (PointCountError("Index::LoadCellFromTempFile"));
	} // if
	return (true);

} // Index::LoadCellFromTempFile

FILE *Index::OpenTempFile(void)
{

	m_tempFileStarted = 0;
	m_tempFileWrittenBytes = 0;
	return (m_tempFile = fopen(m_tempFileName.c_str(), "wb+"));
    
} // Index::OpenTempFile

void Index::CloseTempFile(void)
{

	if (m_tempFile)
	{
		fclose(m_tempFile);
		remove(m_tempFileName.c_str());
	} // if
	m_tempFile = 0;
	m_tempFileWrittenBytes = 0;
    
} // Index::CloseTempFile

bool Index::SaveIndexInLASFile(void)
{
	try {
		Writer writer(std::string(m_ofs), m_idxheader);
		m_reader->seek(0);
		while (m_reader->readNextPoint())
		{
			Point const& CurPt = m_reader->getPoint();
			if (! writer.writePoint(CurPt))
				return (OutputFileError("Index::SaveIndexInLASFile"));
		} // while
	} // try
	catch (std::runtime_error const&) {
		return (OutputFileError("Index::saveIndexInLASFile"));
	} // catch
	return true;
} // Index::SaveIndexInLASFile

bool Index::SaveIndexInStandAloneFile(void)
{
	try {
		Writer writer(std::string(m_ofs), m_idxheader);
		/* test block - uncommenting this makes it just like above version with included points
		m_reader->Seek(0);
		while (m_reader->readNextPoint())
		{
			Point const& CurPt = m_reader->getPoint();
			if (! writer.writePoint(CurPt))
				return (OutputFileError("Index::saveIndexInLASFile"));
		} // while
		*/
	} // try
	catch (std::runtime_error const&) {
		return (OutputFileError("Index::SaveIndexInStandAloneFile"));
	} // catch
	return true;
} // Index::SaveIndexInStandAloneFile

bool Index::FileError(const char *Reporter)
{

	CloseTempFile();
	if (m_debugOutputLevel)
		fprintf(m_debugger, "File i/o error, %s\n", Reporter);
	return false;

} // Index::FileError

bool Index::InputFileError(const char *Reporter) const
{

	if (m_debugOutputLevel)
		fprintf(m_debugger, "Input file i/o error, %s\n", Reporter);
	return false;

} // Index::InputFileError

bool Index::OutputFileError(const char *Reporter) const
{

	if (m_debugOutputLevel)
		fprintf(m_debugger, "Output file i/o error, %s\n", Reporter);
	return false;

} // Index::OutputFileError

bool Index::DebugOutputError(const char *Reporter) const
{

	if (m_debugOutputLevel)
		fprintf(m_debugger, "Debug output error, %s\n", Reporter);
	return false;

} // Index::DebugOutputError

bool Index::PointCountError(const char *Reporter) const
{

	if (m_debugOutputLevel)
		fprintf(m_debugger, "Point checksum error, %s\n", Reporter);
	return false;

} // Index::PointCountError

bool Index::PointBoundsError(const char *Reporter) const
{

	if (m_debugOutputLevel)
		fprintf(m_debugger, "Point out of bounds error, %s\n", Reporter);
	return false;

} // Index::PointBoundsError

bool Index::MemoryError(const char *Reporter) const
{

	if (m_debugOutputLevel)
		fprintf(m_debugger, "Memory error, %s\n", Reporter);
	return false;

} // Index::MemoryError

bool Index::InitError(const char *Reporter) const
{

	if (m_debugOutputLevel)
		fprintf(m_debugger, "Index creation failure, %s\n", Reporter);
	return false;

} // Index::InitError

bool Index::InputBoundsError(const char *Reporter) const
{

	if (m_debugOutputLevel)
		fprintf(m_debugger, "Input file has inappropriate bounds, %s\n", Reporter);
	return false;

} // Index::InputBoundsError

#define LIBHSL_INDEX_DEBUGCELLBINS 20

bool Index::OutputCellStats(IndexCellDataBlock& CellBlock) const
{
	uint32_t MaxPointsPerCell = 0;

	for (uint32_t x = 0; x < m_cellsX; ++x)
	{
		for (uint32_t y = 0; y < m_cellsY; ++y)
		{
			uint32_t PointsThisCell = CellBlock[x][y].GetNumPoints();
			if (PointsThisCell > MaxPointsPerCell)
				MaxPointsPerCell = PointsThisCell;
		} // for
	} // for
	
	std::vector<uint32_t> CellPopulation(LIBHSL_INDEX_DEBUGCELLBINS);

	for (uint32_t x = 0; x < m_cellsX; ++x)
	{
		for (uint32_t y = 0; y < m_cellsY; ++y)
		{
			uint32_t PointsThisCell = CellBlock[x][y].GetNumPoints();
			uint32_t BinThisCell = (uint32_t )(LIBHSL_INDEX_DEBUGCELLBINS * (double)PointsThisCell / (double)MaxPointsPerCell);
			if (BinThisCell >= LIBHSL_INDEX_DEBUGCELLBINS)
				BinThisCell = LIBHSL_INDEX_DEBUGCELLBINS - 1;
			++CellPopulation[BinThisCell];
		} // for
	} // for
	
	fprintf(m_debugger, "Max points per cell %d\n", MaxPointsPerCell);
	OutputCellGraph(CellPopulation, MaxPointsPerCell);
	// no way for this to fail at this time but allow for future modifications by having a return value
	return true;
	
} // Index::OutputCellStats

bool Index::OutputCellGraph(std::vector<uint32_t> CellPopulation, uint32_t MaxPointsPerCell) const
{
	
	for (uint32_t i = 0; i < CellPopulation.size(); ++i)
	{
		fprintf(m_debugger,"Bin %2d (%4d-%4d)... Cells in point range bin %d\n", i, (i * MaxPointsPerCell / LIBHSL_INDEX_DEBUGCELLBINS),
			((i + 1)* MaxPointsPerCell / LIBHSL_INDEX_DEBUGCELLBINS), CellPopulation[i]);
	} // for
	// no way for this to fail at this time but allow for future modifications by having a return value
	return true;
	
} // Index::OutputCellGraph

const char *Index::GetIndexAuthorStr(void) const
{
	return (m_indexAuthor.c_str());
} // Index::GetIndexAuthorStr

const char *Index::GetIndexCommentStr(void) const
{
	return (m_indexComment.c_str());
} // Index::GetIndexAuthorStr

const char *Index::GetIndexDateStr(void) const
{
	return (m_indexDate.c_str());
} // Index::GetIndexAuthorStr

IndexData::IndexData(void)
{
	SetValues();
} // IndexData::IndexData

IndexData::IndexData(Index const& index)
{
	SetValues();
	m_reader = index.GetReader();
	m_idxreader = index.GetIndexReader();
	m_filter = index.GetBounds();
    m_debugOutputLevel = index.GetDebugOutputLevel();
	m_tempFileName = index.GetTempFileName() ? index.GetTempFileName(): "";
	m_indexAuthor = index.GetIndexAuthorStr() ? index.GetIndexAuthorStr(): "";
	m_indexComment = index.GetIndexCommentStr() ? index.GetIndexCommentStr(): "";
	m_indexDate = index.GetIndexDateStr() ? index.GetIndexDateStr(): "";
	m_cellSizeZ = index.GetCellSizeZ();
	m_debugger = index.GetDebugger() ? index.GetDebugger(): stderr;
	m_readOnly = index.GetReadOnly();
	m_forceNewIndex = index.GetForceNewIndex();
	if (index.GetMaxMemoryUsage() > 0)
		m_maxMemoryUsage = index.GetMaxMemoryUsage();
	else
		m_maxMemoryUsage = LIBHSL_INDEX_MAXMEMDEFAULT;
	if (m_maxMemoryUsage < LIBHSL_INDEX_MINMEMDEFAULT)
		m_maxMemoryUsage = LIBHSL_INDEX_MINMEMDEFAULT;
	m_indexValid = index.IndexReady();
} // IndexData::IndexData

IndexData::IndexData(IndexData const& other)
{
	Copy(other);
} // IndexData::IndexData

IndexData& IndexData::operator=(IndexData const& rhs)
{
    Copy(rhs);
	return (*this);
} // IndexData::operator=

void IndexData::Copy(IndexData const& other)
{
    if (&other != this)
    {
		m_reader = other.m_reader;
		m_idxreader = other.m_idxreader;
		m_iterator = other.m_iterator;
		m_ifs = other.m_ifs;
		m_ofs = other.m_ofs;
		m_tempFileName = other.m_tempFileName;
		m_indexAuthor = other.m_indexAuthor;
		m_indexComment = other.m_indexComment;
		m_indexDate = other.m_indexDate;
		m_cellSizeZ = other.m_cellSizeZ;
		m_maxMemoryUsage = other.m_maxMemoryUsage;
		m_debugOutputLevel = other.m_debugOutputLevel;
		m_readOnly = other.m_readOnly;
		m_writestandaloneindex = other.m_writestandaloneindex;
		m_forceNewIndex = other.m_forceNewIndex;
		m_debugger = other.m_debugger;
		m_indexValid = other.m_indexValid;
		m_noFilterX = other.m_noFilterX;
		m_noFilterY = other.m_noFilterY;
		m_noFilterZ = other.m_noFilterZ;
		m_filter = other.m_filter;
		m_LowXCellCompletelyIn = other.m_LowXCellCompletelyIn;
		m_HighXCellCompletelyIn = other.m_HighXCellCompletelyIn;
		m_LowYCellCompletelyIn = other.m_LowYCellCompletelyIn;
		m_HighYCellCompletelyIn = other.m_HighYCellCompletelyIn;
		m_LowZCellCompletelyIn = other.m_LowZCellCompletelyIn;
		m_HighZCellCompletelyIn = other.m_HighZCellCompletelyIn;
		m_LowXBorderCell = other.m_LowXBorderCell;
		m_HighXBorderCell = other.m_HighXBorderCell;
		m_LowYBorderCell = other.m_LowYBorderCell;
		m_HighYBorderCell = other.m_HighYBorderCell;
		m_LowZBorderCell = other.m_LowZBorderCell;
		m_HighZBorderCell = other.m_HighZBorderCell;
		m_LowXBorderPartCell = other.m_LowXBorderPartCell;
		m_HighXBorderPartCell = other.m_HighXBorderPartCell;
		m_LowYBorderPartCell = other.m_LowYBorderPartCell;
		m_HighYBorderPartCell = other.m_HighYBorderPartCell;
	} // if
} // IndexData::Copy

void IndexData::SetValues(void)
{
	m_reader = 0;
	m_idxreader = 0;
	m_iterator = 0;
	m_ifs = 0;
	m_ofs = 0;
	m_tempFileName = 0;
	m_indexAuthor = 0;
	m_indexComment = 0;
	m_indexDate = 0;
	m_cellSizeZ = 0.0;
	m_maxMemoryUsage = 0;
	m_debugOutputLevel = 0;
	m_readOnly = false;
	m_writestandaloneindex = false;
	m_forceNewIndex = false;
	m_debugger = 0;
	m_indexValid = false;
	m_noFilterX = m_noFilterY = m_noFilterZ = false;
    m_LowXCellCompletelyIn = m_HighXCellCompletelyIn = m_LowYCellCompletelyIn = m_HighYCellCompletelyIn = 
		m_LowZCellCompletelyIn = m_HighZCellCompletelyIn =
		m_LowXBorderCell = m_HighXBorderCell = m_LowYBorderCell = m_HighYBorderCell = 
		m_LowZBorderCell = m_HighZBorderCell = 0;
	m_LowXBorderPartCell = m_HighXBorderPartCell = m_LowYBorderPartCell = m_HighYBorderPartCell = 0.0;
} // IndexData::SetValues

bool IndexData::SetInitialValues(const char *ifs, Reader *reader, const char *ofs, Reader *idxreader, 
	const char *tmpfilenme, const char *indexauthor, 
	const char *indexcomment, const char *indexdate, double zbinht, 
	uint32_t maxmem, int debugoutputlevel, bool readonly, bool writestandaloneindex, 
	bool forcenewindex, FILE *debugger)
{

	m_ifs = ifs;
	m_ofs = ofs;
	m_reader = reader;
	m_idxreader = idxreader;
	m_iterator = 0;
	m_tempFileName = tmpfilenme;
	m_indexAuthor = indexauthor;
	m_indexComment = indexcomment;
	m_indexDate = indexdate;
	m_cellSizeZ = zbinht;
	m_maxMemoryUsage = maxmem;
	m_debugOutputLevel = debugoutputlevel;
	m_readOnly = readonly;
	m_writestandaloneindex = writestandaloneindex;
	m_forceNewIndex = forcenewindex;
	m_debugger = debugger;
	m_indexValid = false;
    m_LowXCellCompletelyIn = m_HighXCellCompletelyIn = m_LowYCellCompletelyIn = m_HighYCellCompletelyIn = 
		m_LowZCellCompletelyIn = m_HighZCellCompletelyIn =
		m_LowXBorderCell = m_HighXBorderCell = m_LowYBorderCell = m_HighYBorderCell = 
		m_LowZBorderCell = m_HighZBorderCell = 0;
	m_LowXBorderPartCell = m_HighXBorderPartCell = m_LowYBorderPartCell = m_HighYBorderPartCell = 0.0;
	return (m_reader || m_ifs);
	
} // IndexData::SetInitialValues

bool IndexData::SetBuildEmbedValues(Reader *reader, const char *ofs, const char *tmpfilenme, const char *indexauthor, 
	const char *indexcomment, const char *indexdate, double zbinht, 
	uint32_t maxmem, int debugoutputlevel, FILE *debugger)
{

	m_ifs = 0;
	m_ofs = ofs;
	m_reader = reader;
	m_idxreader = 0;
	m_iterator = 0;
	m_tempFileName = tmpfilenme;
	m_indexAuthor = indexauthor;
	m_indexComment = indexcomment;
	m_indexDate = indexdate;
	m_cellSizeZ = zbinht;
	m_maxMemoryUsage = maxmem;
	m_debugOutputLevel = debugoutputlevel;
	m_readOnly = false;
	m_writestandaloneindex = false;
	m_forceNewIndex = true;
	m_debugger = debugger;
	m_indexValid = false;
	return (m_reader && m_ofs && m_tempFileName);
	
} // IndexData::SetBuildEmbedValues

bool IndexData::SetBuildAloneValues(Reader *reader, const char *ofs, const char *tmpfilenme, const char *indexauthor, 
	const char *indexcomment, const char *indexdate, double zbinht, 
	uint32_t maxmem, int debugoutputlevel, FILE *debugger)
{

	m_ifs = 0;
	m_ofs = ofs;
	m_reader = reader;
	m_idxreader = 0;
	m_iterator = 0;
	m_tempFileName = tmpfilenme;
	m_indexAuthor = indexauthor;
	m_indexComment = indexcomment;
	m_indexDate = indexdate;
	m_cellSizeZ = zbinht;
	m_maxMemoryUsage = maxmem;
	m_debugOutputLevel = debugoutputlevel;
	m_readOnly = false;
	m_writestandaloneindex = true;
	m_forceNewIndex = true;
	m_debugger = debugger;
	m_indexValid = false;
	return (m_reader && m_ofs && m_tempFileName);
	
} // IndexData::SetBuildAloneValues

bool IndexData::SetReadEmbedValues(Reader *reader, int debugoutputlevel, FILE *debugger)
{

	m_ifs = 0;
	m_ofs = 0;
	m_reader = reader;
	m_idxreader = 0;
	m_iterator = 0;
	m_tempFileName = 0;
	m_indexAuthor = 0;
	m_indexComment = 0;
	m_indexDate = 0;
	m_cellSizeZ = 0.0;
	m_maxMemoryUsage = 0;
	m_debugOutputLevel = debugoutputlevel;
	m_readOnly = true;
	m_writestandaloneindex = false;
	m_forceNewIndex = false;
	m_debugger = debugger;
	m_indexValid = false;
	return (m_reader ? true: false);
	
} // IndexData::SetReadEmbedValues

bool IndexData::SetReadAloneValues(Reader *reader, Reader *idxreader, int debugoutputlevel, FILE *debugger)
{

	m_ifs = 0;
	m_ofs = 0;
	m_reader = reader;
	m_idxreader = idxreader;
	m_iterator = 0;
	m_tempFileName = 0;
	m_indexAuthor = 0;
	m_indexComment = 0;
	m_indexDate = 0;
	m_cellSizeZ = 0.0;
	m_maxMemoryUsage = 0;
	m_debugOutputLevel = debugoutputlevel;
	m_readOnly = true;
	m_writestandaloneindex = false;
	m_forceNewIndex = false;
	m_debugger = debugger;
	m_indexValid = false;
	return (m_reader && m_idxreader);
	
} // IndexData::SetReadAloneValues

bool IndexData::SetReadOrBuildEmbedValues(Reader *reader, const char *ofs, const char *tmpfilenme, const char *indexauthor, 
	const char *indexcomment, const char *indexdate, double zbinht, 
	uint32_t maxmem, int debugoutputlevel, FILE *debugger)
{

	SetBuildEmbedValues(reader, ofs, tmpfilenme, indexauthor, indexcomment, indexdate, zbinht, 
		maxmem, debugoutputlevel, debugger);

	m_forceNewIndex = false;
	return (m_reader && m_ofs && m_tempFileName);
	
} // IndexData::SetBuildEmbedValues

bool IndexData::SetReadOrBuildAloneValues(Reader *reader, const char *ofs, const char *tmpfilenme, const char *indexauthor, 
	const char *indexcomment, const char *indexdate, double zbinht, 
	uint32_t maxmem, int debugoutputlevel, FILE *debugger)
{

	SetBuildAloneValues(reader, ofs, tmpfilenme, indexauthor, indexcomment, indexdate, zbinht, 
		maxmem, debugoutputlevel, debugger);

	m_forceNewIndex = false;
	return (m_reader && m_ofs && m_tempFileName);
	
} // IndexData::SetBuildAloneValues

bool IndexData::SetFilterValues(double LowFilterX, double HighFilterX, double LowFilterY, double HighFilterY, 
	double LowFilterZ, double HighFilterZ, Index const& index)
{
	try {
		m_filter = Bounds<double>(LowFilterX, LowFilterY, LowFilterZ, HighFilterX, HighFilterY, HighFilterZ);
		m_filter.verify();
		m_filter.clip(index.GetBounds());
	} // try
	catch (std::runtime_error const&) {
		return (false);
	} // catch verification error
	return (CalcFilterEnablers());
	
} // IndexData::SetFilterValues

bool IndexData::SetFilterValues(Bounds<double> const& src, Index const& index)
{
	try {
		m_filter = src;
		m_filter.verify();
		m_filter.clip(index.GetBounds());
	} // try
	catch (std::runtime_error const&) {
		return (false);
	} // catch verification error
	return (CalcFilterEnablers());
	
} // IndexData::SetFilterValues

bool IndexData::CalcFilterEnablers(void)
{
	if (detail::compare_distance((m_filter.min)(0), (m_filter.max)(0)))
		m_noFilterX = true;
	if (detail::compare_distance((m_filter.min)(1), (m_filter.max)(1)))
		m_noFilterY = true;
	if (detail::compare_distance((m_filter.min)(2), (m_filter.max)(2)))
		m_noFilterZ = true;
	return (! (m_noFilterX && m_noFilterY && m_noFilterZ));
} // IndexData::CalcFilterEnablers

void IndexData::ClampFilterBounds(Bounds<double> const& m_bounds)
{
	m_filter.clip(m_bounds);
} // IndexData::ClampFilterBounds

IndexIterator::IndexIterator(Index *IndexSrc, double LowFilterX, double HighFilterX, double LowFilterY, double HighFilterY, 
	double LowFilterZ, double HighFilterZ, uint32_t ChunkSize)
	: m_indexData(*IndexSrc)
{
	m_index = IndexSrc;
	m_indexData.m_filter = Bounds<double>(LowFilterX, LowFilterY, LowFilterZ, HighFilterX, HighFilterY, HighFilterZ);
	m_chunkSize = ChunkSize;
	m_advance = 0;
	ResetPosition();
} // IndexIterator::IndexIterator

IndexIterator::IndexIterator(Index *IndexSrc, IndexData const& IndexDataSrc, uint32_t ChunkSize)
	: m_indexData(IndexDataSrc)
{
	m_index = IndexSrc;
	m_chunkSize = ChunkSize;
	m_advance = 0;
	ResetPosition();
} // IndexIterator::IndexIterator

IndexIterator::IndexIterator(Index *IndexSrc, Bounds<double> const& BoundsSrc, uint32_t ChunkSize)
{
	m_index = IndexSrc;
	m_indexData = IndexData(*IndexSrc);
	m_indexData.m_filter = BoundsSrc;
	m_chunkSize = ChunkSize;
	m_advance = 0;
	ResetPosition();
} // IndexIterator::IndexIterator

IndexIterator::IndexIterator(IndexIterator const& other)
{
	Copy(other);
} // IndexIterator::IndexIterator

IndexIterator& IndexIterator::operator=(IndexIterator const& rhs)
{
	Copy(rhs);
	return (*this);
} // IndexIterator::operator=

void IndexIterator::Copy(IndexIterator const& other)
{
	if (&other != this)
	{
		m_index = other.m_index;
		m_indexData = other.m_indexData;
		m_chunkSize = other.m_chunkSize;
		m_advance = other.m_advance;
		m_curVLR = other.m_curVLR;
		m_curCellStartPos = other.m_curCellStartPos;
		m_curCellX = other.m_curCellX;
		m_curCellY = other.m_curCellY;
		m_totalPointsScanned = other.m_totalPointsScanned;
		m_ptsScannedCurCell = other.m_ptsScannedCurCell;
		m_ptsScannedCurVLR = other.m_ptsScannedCurVLR;
		m_conformingPtsFound = other.m_conformingPtsFound;
	} // if
} // IndexIterator::Copy

void IndexIterator::ResetPosition(void)
{
	m_curVLR = m_curCellStartPos = m_curCellX = m_curCellY = 0;
	m_totalPointsScanned = m_ptsScannedCurCell = m_ptsScannedCurVLR = 0;
	m_conformingPtsFound = 0;
} // IndexIterator::ResetPosition

const std::vector<uint32_t>& IndexIterator::operator()(int32_t n)
{
	if (n <= 0)
	{
		ResetPosition();
		m_advance = 1;
	} // if
	else if ((uint32_t)n < m_conformingPtsFound)
	{
		ResetPosition();
		m_advance = n + 1;
	} // if
	else
	{
		m_advance = n - m_conformingPtsFound + 1;
	} // else
	m_indexData.SetIterator(this);
	return (m_index->Filter(m_indexData));
} // IndexIterator::operator++

const std::vector<uint32_t>& IndexIterator::advance(int32_t n)
{
	if (n > 0)
		--n;
	return ((*this)(m_conformingPtsFound + n));
} // IndexIterator::advance

} // namespace hsl

