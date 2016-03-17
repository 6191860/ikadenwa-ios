//
//  Room.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "Room.h"

using namespace nwr;
using namespace nwr::jsrtc;

@implementation Room

- (instancetype)initWithDelegate:(NSObject<RoomDelegate> *)delegate
{
    self = [super init];
    if (!self) { return nil; }
    
    _delegate = delegate;
    _isLoggedIn = NO;
    _easyrtcid = nil;
    _users = [NSMutableArray array];
    _documentTitle = @"";
    _roomName = @"";
    _userName = @"";
    _localStream = nullptr;
    _localMonitor = NO;

    //    this.inputVolume = '';
//    ko.track(this);
    
    return self;
}

- (User *)userForEasyrtcid:(NSString *)easyrtcid {
    for (int i = 0; i < _users.count; i++) {
        if ([_users[i].easyrtcid isEqualToString:easyrtcid]) {
            return _users[i];
        }
    }
    return nil;
}

- (void)setIsLoggedIn:(BOOL)newValue {
    BOOL oldValue = _isLoggedIn;
    _isLoggedIn = newValue;
    if (oldValue != newValue) {
        if (newValue) {
            [_delegate roomOnLoggedIn];
        }
    }
}

- (void)setUsers:(NSArray<User *> *)newUsers {
    NSArray<User *> * oldUsers = _users;
    NSMutableArray<User *> * users = [NSMutableArray array];
    
    for (int newIndex = 0; newIndex < newUsers.count; newIndex++) {
        User * newUser = newUsers[newIndex];
        int oldIndex = -1;
        User * oldUser = UserFindByEasyrtcid(oldUsers, &oldIndex, newUser.easyrtcid);
        if (!oldUser) {
            newUser.view = [self createUserViewAt:newIndex];
            [users addObject:newUser];
        } else {
            [users addObject:oldUser];
            if (oldIndex != newIndex) {
                [self moveUserView:oldUser.view to:newIndex];
            }
        }
    }
    
    for (int oldIndex = 0; oldIndex < oldUsers.count; oldIndex++) {
        User * oldUser = oldUsers[oldIndex];
        User * newUser = UserFindByEasyrtcid(newUsers, nil, oldUser.easyrtcid);
        if (!newUser) {
            [self deleteUserView:oldUser.view];
        }
    }
    
    _users = users;
}

- (void)activateWithRoomName:(NSString *)roomName {
    _roomName = roomName;
    _easyrtcid = nil;
    _users = [NSMutableArray array];
    _documentTitle = roomName;
    _userName = @"ikadenwa-ios-test"; // TODO
    [self join];
}

- (void)deactivate {
    if (_easyrtcid) {
        [self _logout];
    }
}

- (void)join {
    auto easyrtc = _delegate.easyrtc;
    
    easyrtc->set_user_name(ToString(_userName));
    
    easyrtc->set_room_occupant_listener([self](const Optional<std::string> & roomName,
                                               const std::map<std::string, Any> & occupants,
                                               const Any & isPrimary)
                                        {
                                            if (*roomName != ToString(_roomName)) {
                                                return;
                                            }
                                            
                                            self.isLoggedIn = YES;
                                            
                                            NSMutableArray<User *> * newUsers = [NSMutableArray array];
                                            
                                            User * myUser = UserFindByEasyrtcid(_users, nil, _easyrtcid);
                                            if (!myUser) {
                                                myUser = [[User alloc] initWithDelegate:[_delegate userDelegate]
                                                                              easyrtcId:_easyrtcid
                                                                                   name:_userName
                                                                                 joined:0.0];
                                                myUser.isMyself = YES;
                                            }
                                            [newUsers addObject:myUser];
                                            
                                            
                                            for (const std::string & easyrtcid : Keys(occupants)) {
                                                User * newUser = UserFindByEasyrtcid(_users, nil, ToNSString(easyrtcid));
                                                if (!newUser) {
                                                    const Any & userData = occupants.at(easyrtcid);
                                                    newUser = [[User alloc] initWithDelegate:[_delegate userDelegate]
                                                                                   easyrtcId:ToNSString(userData.GetAt("easyrtcid").AsString().value())
                                                                                        name:ToNSString(userData.GetAt("username").AsString().value())
                                                                                      joined:userData.GetAt("roomJoinTime").AsDouble().value()];
                                                }
                                                [newUsers addObject:newUser];
                                            }
                                            [newUsers sortUsingComparator:^NSComparisonResult(User * a, User * b){
                                                if (a.isMyself != b.isMyself) {
                                                    return a.isMyself ? NSOrderedAscending : NSOrderedDescending;
                                                }
                                                if (a.joined != b.joined) {
                                                    return a.joined < b.joined ? NSOrderedAscending : NSOrderedDescending;
                                                }
                                                return NSOrderedSame;
                                            }];
                                            
                                            self.users = newUsers;
                                        });
    
    easyrtc->SetPeerListener(None(), None(),
                             [self](const std::string & easyrtcid,
                                const std::string & messageType,
                                const Any & content,
                                const Any & targeting)
                             {
                                 if (messageType == "mute") {
                                     User * user = [self userForEasyrtcid:ToNSString(easyrtcid)];
                                     [user muteWithFlag:content.GetAt("mute").AsBoolean().value()];
                                 }
                             });
    easyrtc->set_stream_acceptor([self](const std::string & easyrtcid,
                                        const std::shared_ptr<MediaStream> & stream,
                                        const std::string & stream_name)
                                 {
                                     User * user = [self userForEasyrtcid:ToNSString(easyrtcid)];
                                     [user embedStream:stream];
                                 });
    easyrtc->set_on_stream_closed([self](const std::string & easyrtcid,
                                         const std::shared_ptr<MediaStream> & stream,
                                         const std::string & stream_name)
                                  {
                                      User * user = [self userForEasyrtcid:ToNSString(easyrtcid)];
                                      [user removeStream];
                                  });
    
    easyrtc->set_accept_checker([self](const std::string & easyrtcid,
                                       const std::function<void (bool,
                                                                 const Optional<std::vector<std::string>> &)> & callback)
                                {
                                    Timer::Create(TimeDuration(0.01), [callback]() {
                                        callback(true, None());
                                    });
                                });
    //bug in original
    easyrtc->set_on_error([self](const Any & error) {
        std::string errorCode = error.GetAt("errorCode").AsString().value();
        std::string message = error.GetAt("errorText").AsString().value();
        [self handleErrorWithCode:ToNSString(errorCode) text:ToNSString(message)];
    });
    easyrtc->JoinRoom(ToString(_roomName),
                      nullptr,
                      [](const std::string & room_name)
                      {
                          
                      },
                      [self]
                      (const std::string & errorCode, const std::string & message, const std::string & room_name)
                      {
                          [self handleErrorWithCode:ToNSString(errorCode)
                                               text:ToNSString(message)];
                      });
    easyrtc->EnableAudio(true);
    easyrtc->EnableVideo(false);
    easyrtc->EnableVideoReceive(false);
    easyrtc->InitMediaSource(None(),
                             [self, easyrtc](const std::shared_ptr<MediaStream> & stream)
                             {
                                 easyrtc->Connect("squid",
                                                  [self, easyrtc](const std::string & easyrtcid)
                                                  {
                                                      _easyrtcid = ToNSString(easyrtcid);
                                                      
                                                      _localStream = easyrtc->GetLocalStream(None());
                                                      
                                                      
//                                                      var audio = document.createElement('audio');
//                                                      audio.setAttribute('id', 'localStream');
//                                                      audio.autoplay = true;
//                                                      audio.muted = true;
//                                                      document.body.appendChild(audio);
//                                                      easyrtc.setVideoObjectSrc(audio, _this.localStream);
                                                      
                                                      _localMonitor = false;
                                                  },
                                                  [self](const std::string & code, const std::string & text)
                                                  {
                                                      [self handleErrorWithCode:ToNSString(code) text:ToNSString(text)];
                                                  });
                             },
                             [self](const std::string & code, const std::string & message)
                             {
                                 [self handleErrorWithCode:ToNSString(code) text:ToNSString(message)];
                             });
}

- (void)muteAll {
    for (User * user : _users) {
        [user requestMute];
    }
}

- (void)unmuteAll {
    for (User * user : _users) {
        [user requestUnmute];
    }
}

- (void)toggleMonitor {
    //TODO
//    easyrtc.muteVideoObject(document.getElementById('localStream'), this.localMonitor);
    _localMonitor = !_localMonitor;
}

- (void)_logout {
    if (_localMonitor) {
        [self toggleMonitor];
    }
    
    _delegate.easyrtc->HangupAll();
    _delegate.easyrtc->Disconnect();

    _easyrtcid = nil;
    self.isLoggedIn = NO;
    
//    var localStream = document.getElementById('localStream');
//    localStream.parentElement.removeChild(localStream);
//    this.localStream = null;
//    sessionStorage.setItem('userName', '');
}

- (void)logout {
    [self _logout];
//    TODO
//    router.navigate('');
}

- (void)handleErrorWithCode:(NSString *)code
                       text:(NSString *)text
{
    [_delegate roomOnErrorWithCode:code text:text];
}

- (UserPanel *)createUserViewAt:(int)index {
    UserPanel * userPanel = [UserPanel load];
    [[_delegate roomScrollView] addSubview:userPanel];
    userPanel.frame = [_delegate roomUserPanelFrameAt:index];
    userPanel.alpha = 0.0;
    [UIView animateWithDuration:[_delegate animationDuration] animations:^{
        userPanel.alpha = 1.0;
    }];
    return userPanel;
}

- (void)deleteUserView:(UserPanel *)view {
    [UIView animateWithDuration:[_delegate animationDuration] animations:^{
        view.alpha = 0.0;
    } completion:^(BOOL finished){
        [view removeFromSuperview];
    }];
}

- (void)moveUserView:(UserPanel *)view to:(int)index {
    CGRect frame = [_delegate roomUserPanelFrameAt:index];
    [UIView animateWithDuration:[_delegate animationDuration] animations:^{
        view.frame = frame;
    }];
}

@end
