/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_WEBTRANSPORT_PARENT_WEBTRANSPORTPARENT_H_
#define DOM_WEBTRANSPORT_PARENT_WEBTRANSPORTPARENT_H_

#include "ErrorList.h"
#include "mozilla/dom/FlippedOnce.h"
#include "mozilla/dom/PWebTransportParent.h"
#include "mozilla/ipc/Endpoint.h"
#include "mozilla/ipc/PBackgroundSharedTypes.h"
#include "nsISupports.h"
#include "nsIPrincipal.h"
#include "nsIWebTransport.h"

namespace mozilla::dom {

enum class WebTransportReliabilityMode : uint8_t;

class WebTransportParent : public PWebTransportParent,
                           public WebTransportSessionEventListener {
  using IPCResult = mozilla::ipc::IPCResult;

 public:
  WebTransportParent() = default;

  NS_DECL_THREADSAFE_ISUPPORTS
  NS_DECL_WEBTRANSPORTSESSIONEVENTLISTENER

  void Create(
      const nsAString& aURL, nsIPrincipal* aPrincipal, const bool& aDedicated,
      const bool& aRequireUnreliable, const uint32_t& aCongestionControl,
      // Sequence<WebTransportHash>* aServerCertHashes,
      Endpoint<PWebTransportParent>&& aParentEndpoint,
      std::function<void(Tuple<const nsresult&, const uint8_t&>)>&& aResolver);

  IPCResult RecvClose(const uint32_t& aCode, const nsACString& aReason);

  IPCResult RecvCreateUnidirectionalStream(
      Maybe<int64_t> aSendOrder,
      CreateUnidirectionalStreamResolver&& aResolver);
  IPCResult RecvCreateBidirectionalStream(
      Maybe<int64_t> aSendOrder, CreateBidirectionalStreamResolver&& aResolver);

  void ActorDestroy(ActorDestroyReason aWhy) override;

 protected:
  virtual ~WebTransportParent();

 private:
  void NotifyRemoteClosed(uint32_t aErrorCode, const nsACString& aReason);

  using ResolveType = Tuple<const nsresult&, const uint8_t&>;
  nsCOMPtr<nsISerialEventTarget> mSocketThread;
  Atomic<bool> mSessionReady{false};

  mozilla::Mutex mMutex{"WebTransportParent::mMutex"};
  std::function<void(ResolveType)> mResolver MOZ_GUARDED_BY(mMutex);
  // This is needed because mResolver is resolved on the background thread and
  // OnSessionClosed is called on the socket thread.
  std::function<void()> mExecuteAfterResolverCallback MOZ_GUARDED_BY(mMutex);
  FlippedOnce<false> mClosed MOZ_GUARDED_BY(mMutex);

  nsCOMPtr<nsIWebTransport> mWebTransport;
  nsCOMPtr<nsIEventTarget> mOwningEventTarget;
};

}  // namespace mozilla::dom

#endif  // DOM_WEBTRANSPORT_PARENT_WEBTRANSPORTPARENT_H_
