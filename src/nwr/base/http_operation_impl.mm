//
//  http_operation_impl.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "http_operation_impl.h"

using namespace nwr;

@interface NWRHttpOperationNSURLSessionDelegateAdapter : NSObject <
NSURLSessionTaskDelegate,
NSURLSessionDataDelegate>

@property(nonatomic, assign) std::function<void()> onSuccess;
@property(nonatomic, assign) std::function<void(const std::string &)> onFailure;
@property(nonatomic, assign) std::function<void(const HttpResponse &)> onResponse;
@property(nonatomic, assign) std::function<void(const Data &)> onReceiveChunk;

@end

@implementation NWRHttpOperationNSURLSessionDelegateAdapter

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
willPerformHTTPRedirection:(NSHTTPURLResponse *)response
newRequest:(NSURLRequest *)request
completionHandler:(void (^)(NSURLRequest * __nullable))completionHandler
{
    completionHandler(request);
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential * __nullable credential))completionHandler
{
    completionHandler(NSURLSessionAuthChallengeUseCredential, nil);
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
needNewBodyStream:(void (^)(NSInputStream * __nullable bodyStream))completionHandler
{
    completionHandler(nil);
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
didSendBodyData:(int64_t)bytesSent
totalBytesSent:(int64_t)totalBytesSent
totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend
{
    
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
didCompleteWithError:(nullable NSError *)error
{
    if (error) {
        std::string error_str([error localizedDescription].UTF8String);
        FuncCall(_onFailure, error_str);
    } else {
        FuncCall(_onSuccess);
    }
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask
didReceiveResponse:(NSURLResponse *)nsResponse
completionHandler:(void (^)(NSURLSessionResponseDisposition disposition))completionHandler
{
    NSHTTPURLResponse * nsHttpResponse = (NSHTTPURLResponse *)nsResponse;
    
    HttpResponse response;
    response.code = static_cast<int>(nsHttpResponse.statusCode);
    NSDictionary * nsFields = nsHttpResponse.allHeaderFields;
    for (NSString * nsKey in nsFields) {
        NSString * nsValue = nsFields[nsKey];
        std::string key = std::string(nsKey.UTF8String);
        std::string value = std::string(nsValue.UTF8String);
        response.headers[key] = value;
    }
    
    FuncCall(_onResponse, response);
    
    completionHandler(NSURLSessionResponseAllow);
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask
didBecomeDownloadTask:(NSURLSessionDownloadTask *)downloadTask
{
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask
didBecomeStreamTask:(NSURLSessionStreamTask *)streamTask
{
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask
didReceiveData:(NSData *)nsData;
{
    auto p = static_cast<const uint8_t *>(nsData.bytes);
    int size = static_cast<int>(nsData.length);
    Data data(p, p + size);
    
    FuncCall(_onReceiveChunk, data);
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask
willCacheResponse:(NSCachedURLResponse *)proposedResponse
completionHandler:(void (^)(NSCachedURLResponse * __nullable cachedResponse))completionHandler
{
}

@end

namespace nwr {
    HttpOperationImpl::HttpOperationImpl():
    canceled_(false)
    {}
    
    void HttpOperationImpl::Start(const HttpRequest & request) {
        auto thiz = shared_from_this();
        
        NSString * ns_url_str = [NSString stringWithUTF8String:request.url.c_str()];
        NSMutableURLRequest * ns_request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:ns_url_str]];
        ns_request.HTTPMethod = [NSString stringWithUTF8String:request.method.c_str()];
        NSMutableDictionary * ns_header = [NSMutableDictionary dictionary];
        for (const auto & entry : request.headers) {
            NSString * ns_key = [NSString stringWithUTF8String:entry.first.c_str()];
            NSString * ns_value = [NSString stringWithUTF8String:entry.second.c_str()];
            ns_header[ns_key] = ns_value;
        }
        ns_request.allHTTPHeaderFields = ns_header;
        
        delegate_adapter_ = [[NWRHttpOperationNSURLSessionDelegateAdapter alloc] init];
        
        delegate_adapter_.onSuccess = [thiz](){
            if (thiz->canceled_) { return; }
            
            thiz->response_.data = std::make_shared<Data>(thiz->data_);
            FuncCall(thiz->on_success_, thiz->response_);
            thiz->Cancel();
        };
        delegate_adapter_.onFailure = [thiz](const std::string & error) {
            if (thiz->canceled_) { return; }
            
            FuncCall(thiz->on_failure_, error);
            thiz->Cancel();
        };
        delegate_adapter_.onReceiveChunk = [thiz](const Data & chunk) {
            thiz->data_.insert(thiz->data_.end(), chunk.begin(), chunk.end());
        };
        delegate_adapter_.onResponse = [thiz](const HttpResponse & response) {
            thiz->response_ = response;
        };
        
        NSURLSessionConfiguration * conf = [NSURLSessionConfiguration ephemeralSessionConfiguration];
        session_ = [NSURLSession sessionWithConfiguration:conf
                                                 delegate:delegate_adapter_
                                            delegateQueue:IosTaskQueue::current_queue()->inner_queue()];
        task_ = [session_ dataTaskWithRequest:ns_request];
        [task_ resume];
    }
    
    void HttpOperationImpl::Cancel() {
        if (canceled_) { return; }
        
        if (task_) {
            [task_ cancel];
            task_ = nil;
        }
        if (session_) {
            session_ = nil;
        }
        delegate_adapter_ = nil;
        
        canceled_ = true;
    }

    
}

