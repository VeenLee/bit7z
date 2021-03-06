// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "../include/updatecallback.hpp"

#include <iostream>
#include <string>
//debug includes:
//#include <sstream>
//#include <iomanip>
//end of debug includes

#include "7zip/Common/FileStreams.h"
#include "Common/IntToString.h"

#include "../include/bitpropvariant.hpp"
#include "../include/fsutil.hpp"

using namespace std;
using namespace bit7z;

/* Most of this code is taken from the CUpdateCallback class in Client7z.cpp of the 7z SDK
 * Main changes made:
 *  + Use of std::vector instead of CRecordVector, CObjectVector and UStringVector
 *  + Use of std::wstring instead of UString (see Callback base interface)
 *  + Error messages are not showed (see comments in ExtractCallback)
 *  + The work performed originally by the Init method is now performed by the class constructor
 *  + FSItem class is used instead of CDirItem struct */

const std::wstring kEmptyFileAlias = L"[Content]";

UpdateCallback::UpdateCallback( const BitArchiveCreator& creator, const vector< FSItem >& dirItems ) :
    mVolSize( 0 ),
    mDirItems( dirItems ),
    mCreator( creator ),
    mAskPassword( false ) {
    mNeedBeClosed = false;
    mFailedFiles.clear();
}

UpdateCallback::~UpdateCallback() {
    Finilize();
}

HRESULT UpdateCallback::SetTotal( UInt64 size ) {
    if ( mCreator.totalCallback() ) {
        mCreator.totalCallback()( size );
    }
    return S_OK;
}

HRESULT UpdateCallback::SetCompleted( const UInt64* completeValue ) {
    if ( mCreator.progressCallback() && completeValue != nullptr ) {
        mCreator.progressCallback()( *completeValue );
    }
    return S_OK;
}

STDMETHODIMP UpdateCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) {
    if ( mCreator.ratioCallback() && inSize != nullptr && outSize != nullptr ) {
        mCreator.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

HRESULT UpdateCallback::EnumProperties( IEnumSTATPROPSTG** /* enumerator */ ) {
    return E_NOTIMPL;
}

HRESULT UpdateCallback::GetUpdateItemInfo( UInt32 /* index */, Int32* newData,
                                           Int32* newProperties, UInt32* indexInArchive ) {
    if ( newData != nullptr ) {
        *newData = 1; //= true;
    }
    if ( newProperties != nullptr ) {
        *newProperties = 1; //= true;
    }
    if ( indexInArchive != nullptr ) {
        *indexInArchive = static_cast< uint32_t >( -1 );
    }

    return S_OK;
}

// debug purposes
/*wstring to_string( FILETIME ftime ) {// ISO format, time zone designator Z == zero (UTC)
    SYSTEMTIME utc ;
    ::FileTimeToSystemTime( std::addressof(ftime), std::addressof(utc) );

    std::wostringstream stm;
    const auto w2 = std::setw(2);
    stm << std::setfill(L'0') << std::setw(4) << utc.wYear << L'-' << w2 << utc.wMonth
        << L'-' << w2 << utc.wDay << L' ' << w2 << utc.wYear << L' ' << w2 << utc.wHour
        << L':' << w2 << utc.wMinute << L':' << w2 << utc.wSecond << L'Z' ;
    return stm.str() ;
}*/

HRESULT UpdateCallback::GetProperty( UInt32 index, PROPID propID, PROPVARIANT* value ) {
    BitPropVariant prop;

    if ( propID == kpidIsAnti ) {
        prop = false;
        *value = prop;
        return S_OK;
    }

    const FSItem dirItem = mDirItems[index];

    switch ( propID ) {
        case kpidPath:
            prop = dirItem.inArchivePath();
            break;
        case kpidIsDir:
            prop = dirItem.isDir();
            break;
        case kpidSize:
            prop = dirItem.size();
            break;
        case kpidAttrib:
            prop = dirItem.attributes();
            break;
        case kpidCTime:
            prop = dirItem.creationTime();
            break;
        case kpidATime:
            prop = dirItem.lastAccessTime();
            /*wcout << L"dirItem " << dirItem.name()
                  << " last access time: " << to_string( dirItem.lastAccessTime() ) << endl;*/
            break;
        case kpidMTime:
            prop = dirItem.lastWriteTime();
            break;
    }

    *value = prop;
    return S_OK;
}

HRESULT UpdateCallback::Finilize() {
    if ( mNeedBeClosed ) {
        mNeedBeClosed = false;
    }

    return S_OK;
}

HRESULT UpdateCallback::GetStream( UInt32 index, ISequentialInStream** inStream ) {
    RINOK( Finilize() );
    const FSItem dirItem = mDirItems[index];

    if ( mCreator.fileCallback() ) {
        mCreator.fileCallback()( dirItem.name() );
    }

    if ( dirItem.isDir() ) {
        return S_OK;
    }

    auto* inStreamSpec = new CInFileStream;
    CMyComPtr< ISequentialInStream > inStreamLoc( inStreamSpec );
    wstring path = dirItem.path();

    if ( !inStreamSpec->Open( path.c_str() ) ) {
        DWORD sysError = ::GetLastError();
        mFailedFiles[ path ] = static_cast< HRESULT >( sysError );
        // if (systemError == ERROR_SHARING_VIOLATION)
        {
            mErrorMessage = L"WARNING: Can't open file";
            // PrintString(NError::MyFormatMessageW(systemError));
            return S_FALSE;
        }
        // return sysError;
    }

    *inStream = inStreamLoc.Detach();
    return S_OK;
}

HRESULT UpdateCallback::SetOperationResult( Int32 /* operationResult */ ) {
    mNeedBeClosed = true;
    return S_OK;
}

HRESULT UpdateCallback::GetVolumeSize( UInt32 /*index*/, UInt64* size ) {
    if ( mVolSize == 0 ) return S_FALSE;

    *size = mVolSize;
    return S_OK;
}

HRESULT UpdateCallback::GetVolumeStream( UInt32 index, ISequentialOutStream** volumeStream ) {
    wstring res = ( index < 9 ? L"00" : index < 99 ? L"0" : L"" ) + to_wstring( index + 1 );

    wstring fileName = mVolName + L'.' + res;// + mVolExt;
    auto* streamSpec = new COutFileStream;
    CMyComPtr< ISequentialOutStream > streamLoc( streamSpec );

    if ( !streamSpec->Create( fileName.c_str(), false ) ) {
        return ::GetLastError();
    }

    *volumeStream = streamLoc.Detach();
    return S_OK;
}

HRESULT UpdateCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password ) {
    if ( !mCreator.isPasswordDefined() ) {
        if ( mAskPassword ) {
            // You can ask real password here from user
            // Password = GetPassword(OutStream);
            // PasswordIsDefined = true;
            mErrorMessage = L"Password is not defined";
            return E_ABORT;
        }
    }

    *passwordIsDefined = ( mCreator.isPasswordDefined() ? 1 : 0 );
    return StringToBstr( mCreator.password().c_str(), password );
}
