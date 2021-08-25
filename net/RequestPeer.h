// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_CHILD_REQUEST_PEER_H_
#define CONTENT_PUBLIC_CHILD_REQUEST_PEER_H_

#include <string>
#include "third_party/WebKit/Source/wtf/OwnPtr.h"
#include "third_party/WebKit/Source/wtf/PassOwnPtr.h"
#include "third_party/WebKit/Source/wtf/PassRefPtr.h"

namespace net {

struct ResourceResponseInfo;

// This is implemented by our custom resource loader within content. The Peer
// and it's bridge should have identical lifetimes as they represent each end of
// a communication channel.
//
// These callbacks mirror net::URLRequest::Delegate and the order and
// conditions in which they will be called are identical. See url_request.h
// for more information.
class RequestPeer {
public:
    // This class represents data gotten from the Browser process. Each data
    // consists of |payload|, |length| and |encodedLength|. The payload is
    // valid only when the data instance is valid.
    // In order to work with Chrome resource loading IPC, it is desirable to
    // reclaim data in FIFO order in a RequestPeer in terms of performance.
    // |payload|, |length| and |encodedLength| functions are thread-safe, but
    // the data object itself must be destroyed on the original thread.
    class ReceivedData {
    public:
        virtual ~ReceivedData() {}
        virtual const char* payload() const = 0;
        virtual int length() const = 0;
        // The encodedLength is the length of the encoded data transferred
        // over the network, which could be different from data length (e.g. for
        // gzipped content).
        virtual int encodedLength() const = 0;
    };

    // A ThreadSafeReceivedData can be deleted on ANY thread.
    class ThreadSafeReceivedData : public ReceivedData {};

    // Called as upload progress is made.
    // note: only for requests with upload progress enabled.
    virtual void OnUploadProgress(uint64 position, uint64 size) = 0;

    // Called when a redirect occurs.  The implementation may return false to
    // suppress the redirect.  The ResourceResponseInfo provides information about
    // the redirect response and the RedirectInfo includes information about the
    // request to be made if the method returns true.
    //virtual bool OnReceivedRedirect(const net::RedirectInfo& redirect_info, const ResourceResponseInfo& info) = 0;

    // Called when response headers are available (after all redirects have
    // been followed).
    virtual void OnReceivedResponse(const ResourceResponseInfo& info) = 0;

    // Called when a chunk of response data is downloaded.  This method may be
    // called multiple times or not at all if an error occurs.  This method is
    // only called if RequestInfo::download_to_file was set to true, and in
    // that case, OnReceivedData will not be called.
    // The encoded_data_length is the length of the encoded data transferred
    // over the network, which could be different from data length (e.g. for
    // gzipped content).
    virtual void OnDownloadedData(int len, int encodedDataLength) = 0;

    // Called when a chunk of response data is available. This method may
    // be called multiple times or not at all if an error occurs.
    virtual void OnReceivedData(PassOwnPtr<ReceivedData> data) = 0;

    // Called when metadata generated by the renderer is retrieved from the
    // cache. This method may be called zero or one times.
    virtual void OnReceivedCachedMetadata(const char* data, int len) {}

    // Called when the response is complete.  This method signals completion of
    // the resource load.
    virtual void OnCompletedRequest(int errorCode,
        bool wasIgnoredByHandler,
        bool staleCopyInCache,
        const std::string& securityInfo,
        const double& completionTime,
        int64 totalTransferSize) = 0;

    // This is a combined notification of
    //  - OnReceivedResponse,
    //  - OnReceivedData and
    //  - OnCompletedRequest.
    // Unlike OnReceivedData, |data| can be null.
    // This method is introduced to avoid repetitive method calls which might
    // lead to use-after-free issues. See https://crbug.com/485413,
    // https://crbug.com/507170.
    // TODO(yhirano): Fix the RequestPeer lifecycle problem and remove this
    // function.
    virtual void OnReceivedCompletedResponse(
        const ResourceResponseInfo& info,
        PassOwnPtr<ReceivedData> data,
        int errorCode,
        bool wasIgnoredByHandler,
        bool staleCopyInCache,
        const std::string& securityInfo,
        const double& completionTime,
        int64 totalTransferSize) = 0;

protected:
    virtual ~RequestPeer() {}
};

}  // namespace net

#endif  // CONTENT_PUBLIC_CHILD_REQUEST_PEER_H_