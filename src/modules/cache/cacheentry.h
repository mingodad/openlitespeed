/*****************************************************************************
*    Open LiteSpeed is an open source HTTP server.                           *
*    Copyright (C) 2013 - 2015  LiteSpeed Technologies, Inc.                 *
*                                                                            *
*    This program is free software: you can redistribute it and/or modify    *
*    it under the terms of the GNU General Public License as published by    *
*    the Free Software Foundation, either version 3 of the License, or       *
*    (at your option) any later version.                                     *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program. If not, see http://www.gnu.org/licenses/.      *
*****************************************************************************/
#ifndef CACHEENTRY_H
#define CACHEENTRY_H

#include <lsdef.h>
#include <ceheader.h>
#include <cachehash.h>
#include <util/autostr.h>
#include <util/refcounter.h>

#include <sys/types.h>


#define DateTime_s_curTime  ( DateTime::s_curTime )
//#define DateTime_s_curTime  ( time(NULL) )

//#define CACHE_RESP_HEADER   1

#define CE_UPDATING     (1<<0)
#define CE_STALE        (1<<1)

class CacheEntry : public RefCounter
{
public:
    CacheEntry();

    virtual ~CacheEntry();

    void setLastAccess(long tm)
    {
        m_iLastAccess = tm;
    }
    long getLastAccess() const
    {
        return m_iLastAccess;
    }

    void incHits()
    {
        ++m_iHits;
    }
    long getHits() const
    {
        return m_iHits;
    }

    void setFdStore(int fd)
    {
        m_iFdStore = fd;
    }
    int getFdStore() const
    {
        return m_iFdStore;
    }

    void setStartOffset(off_t off)
    {
        m_startOffset = off;
    }
    off_t getStartOffset() const
    {
        return m_startOffset;
    }

    void setMaxStale(int age)
    {
        m_iMaxStale = age;
    }
    int  getMaxStale() const
    {
        return m_iMaxStale;
    }

    off_t getHeaderSize() const
    {
        return 4 + sizeof(CeHeader) + m_header.m_iKeyLen;
    }


    void setContentLen(int part1, int part2)
    {
        m_header.m_iValPart1Len = part1;
        m_header.m_iValPart2Len = part2;
    }

    int getContentTotalLen() const
    {
        return m_header.m_iValPart1Len + m_header.m_iValPart2Len;
    }

    virtual int loadCeHeader() = 0 ;
    virtual int saveCeHeader() = 0 ;
    virtual int allocate(int size) = 0;
    virtual int releaseTmpResource() = 0;

    int setKey(const CacheHash &hash,
               const char *pUri, int iUriLen,
               const char *pQs, int iQsLen,
               const char *pIp, int iIpLen,
               const char *pCookie, int iCookieLen);

    int verifyKey(
        const char *pUri, int iUriLen,
        const char *pQs, int iQsLen,
        const char *pIp, int iIpLen,
        const char *pCookie, int iCookieLen) const;



    void markReady(int compressed)
    {
        m_header.m_iFlag = (m_header.m_iFlag & ~CeHeader::CEH_IN_CONSTRUCT)
                           | (compressed ? CeHeader::CEH_COMPRESSED : 0) ;
    }

    int isGzipped() const
    {
        return m_header.m_iFlag & CeHeader::CEH_COMPRESSED;
    }

    int isUpdating() const
    {
        return m_header.m_iFlag & CeHeader::CEH_UPDATING;
    }
    void setUpdating(int i)
    {
        m_header.m_iFlag = ((m_header.m_iFlag & ~CeHeader::CEH_UPDATING) |
                            (i ? CeHeader::CEH_UPDATING : 0));
    }

    int isStale() const
    {
        return m_header.m_iFlag & CeHeader::CEH_STALE;
    }

    void setStale(int i)
    {
        m_header.m_iFlag = ((m_header.m_iFlag & ~CeHeader::CEH_STALE) |
                            (i ? CeHeader::CEH_STALE : 0));
    }

    CeHeader &getHeader()
    {
        return m_header;
    }
    AutoStr   &getKey()
    {
        return m_sKey;
    }
    const CacheHash &getHashKey() const
    {
        return m_hashKey;
    }

    void setHashKey(const CacheHash &hash)
    {
        m_hashKey.init(hash);
    }

    int getPart1Offset() const
    {
        return m_startOffset + getHeaderSize();
    }

    int getPart2Offset() const
    {
        return m_startOffset + getHeaderSize() + m_header.m_iValPart1Len;
    }

    time_t getExpireTime() const
    {
        return m_header.m_tmExpire;
    }

    void setFilePath(const char *path)
    {
        m_sFile.setStr(path);
    }
    const char *getFilePath(char *buf, int maxBufLen)
    {
        strcpy(buf, m_sFile.c_str());
        if (isStale())
            strcat(buf, ".S");
        return buf;
    }


private:
    long        m_iLastAccess;
    int         m_iHits;
    int         m_iMaxStale;
    CacheHash   m_hashKey;

    off_t       m_startOffset;
    CeHeader    m_header;
    int         m_iFdStore;
    AutoStr     m_sKey;

    AutoStr     m_sFile;

#ifdef CACHE_RESP_HEADER
public:
    //For fast the parsing speed, cache the header to buffer whose size less than 4KB
    AutoStr2    m_sRespHeader;
#endif
public:
    AutoStr2    m_sPart3Buf;


    LS_NO_COPY_ASSIGN(CacheEntry);
};

#endif
