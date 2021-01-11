//------------------------------------------------------------------------------
// Copyright (c) 2011-2014 by European Organization for Nuclear Research (CERN)
// Author: Michal Simon <michal.simon@cern.ch>
//------------------------------------------------------------------------------
// This file is part of the XRootD software suite.
//
// XRootD is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// XRootD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with XRootD.  If not, see <http://www.gnu.org/licenses/>.
//
// In applying this licence, CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
//------------------------------------------------------------------------------

#ifndef SRC_XRDEC_XRDECREADER_HH_
#define SRC_XRDEC_XRDECREADER_HH_

#include "XrdEc/XrdEcObjCfg.hh"
#include "XrdCl/XrdClZipArchive.hh"

#include <string>
#include <unordered_map>

namespace XrdEc
{
  //---------------------------------------------------------------------------
  // Forward declaration for the internal cache
  //---------------------------------------------------------------------------
  struct block_t;
  //---------------------------------------------------------------------------
  // Buffer for a single chunk of data
  //---------------------------------------------------------------------------
  typedef std::vector<char> buffer_t;
  //---------------------------------------------------------------------------
  // Read callback, to be called with status and number of bytes read
  //---------------------------------------------------------------------------
  typedef std::function<void( const XrdCl::XRootDStatus&, uint32_t )> callback_t;

  //---------------------------------------------------------------------------
  // Reader object for reading erasure coded and striped data
  //---------------------------------------------------------------------------
  class Reader
  {
    friend struct block_t;

    public:
      //-----------------------------------------------------------------------
      //! Constructor
      //!
      //! @param objcfg : configuration for the data object (e.g. number of
      //!                 data and parity stripes)
      //-----------------------------------------------------------------------
      Reader( ObjCfg &objcfg ) : objcfg( objcfg )
      {
      }

      //-----------------------------------------------------------------------
      // Destructor
      //-----------------------------------------------------------------------
      virtual ~Reader();

      //-----------------------------------------------------------------------
      //! Open the erasure coded / striped object
      //!
      //! @param handler : user callback
      //-----------------------------------------------------------------------
      void Open( XrdCl::ResponseHandler *handler );

      //-----------------------------------------------------------------------
      //! Read data from the data object
      //!
      //! @param offset  : offset of the data to be read
      //! @param length  : length of the data to be read
      //! @param buffer  : buffer for the data to be read
      //! @param handler : user callback
      //-----------------------------------------------------------------------
      void Read( uint64_t                offset,
                 uint32_t                length,
                 void                   *buffer,
                 XrdCl::ResponseHandler *handler );

    private:

      //-----------------------------------------------------------------------
      //! Read data from given stripes from given block
      //!
      //! @param blknb  : number of the block
      //! @param strpnb : number of stripe in the block
      //! @param buffer : buffer for the data
      //! @param cb     : callback
      //-----------------------------------------------------------------------
      void Read( size_t blknb, size_t strpnb, buffer_t &buffer, callback_t cb );

      //-----------------------------------------------------------------------
      //! Read metadata for the object
      //!
      //! @param index : placement's index
      //-----------------------------------------------------------------------
      XrdCl::Pipeline ReadMetadata( size_t index );

      //-----------------------------------------------------------------------
      //! Parse metadata from chunk info object
      //!
      //! @param ch : chunk info object returned by a read operation
      //-----------------------------------------------------------------------
      bool ParseMetadata( XrdCl::ChunkInfo &ch );

      typedef std::unordered_map<std::string, std::shared_ptr<XrdCl::ZipArchive>> dataarchs_t;
      typedef std::unordered_map<std::string, buffer_t> metadata_t;
      typedef std::unordered_map<std::string, std::string> urlmap_t;

      ObjCfg                   &objcfg;
      dataarchs_t               dataarchs; //> map URL to ZipArchive object
      metadata_t                metadata;  //> map URL to CD metadata
      urlmap_t                  urlmap;    //> map blknb/strpnb (data chunk) to URL
      std::shared_ptr<block_t>  block;     //> cache for the block we are reading from
  };

} /* namespace XrdEc */

#endif /* SRC_XRDEC_XRDECREADER_HH_ */
