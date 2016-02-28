//
//  media_track_constraint.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/22.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "media_track_constraints.h"

namespace nwr {
namespace jsrtc {
    
    class MediaTrackConstraintAdapter : public webrtc::MediaConstraintsInterface {
    public:
        MediaTrackConstraintAdapter(MediaTrackConstraints & outer):
        outer_(outer)
        {}
        
        virtual ~MediaTrackConstraintAdapter() {
            printf("[MediaTrackConstraintAdapter::dtor(%p)]\n", this);
        }
        
        const webrtc::MediaConstraintsInterface::Constraints & GetMandatory() const override {
            auto & entries = outer_.mandatory().inner_entries();
            printf("[MediaTrackConstraintAdapter::GetMandatory(%p)] %p\n", this, &entries);
            return entries;
        }
        const webrtc::MediaConstraintsInterface::Constraints & GetOptional() const override {
            auto & entries = outer_.optional().inner_entries();
            printf("[MediaTrackConstraintAdapter::GetOptional(%p)] %p\n", this, &entries);
            return entries;
        }
        
        MediaTrackConstraints & outer_;
    };
    
    webrtc::MediaConstraintsInterface::Constraints & MediaTrackConstraintSet::inner_entries() {
        return entries_;
    }
    
    const webrtc::MediaConstraintsInterface::Constraints & MediaTrackConstraintSet::inner_entries() const {
        return entries_;
    }
    
    void MediaTrackConstraintSet::SetEntry(const std::string & key, const std::string & value) {
        RemoveIf(entries_, [key](const webrtc::MediaConstraintsInterface::Constraint & x){
            return x.key == key;
        });
        entries_.push_back(webrtc::MediaConstraintsInterface::Constraint(key, value));
    }
    
    void MediaTrackConstraintSet::SetEntry(const std::string & key, bool value) {
        SetEntry(key,
                 std::string(value ?
                             webrtc::MediaConstraintsInterface::kValueTrue :
                             webrtc::MediaConstraintsInterface::kValueFalse)
                 );
    }
    
    void MediaTrackConstraintSet::SetEntry(const std::string & key, int value) {
        SetEntry(key, Format("%d", value));
    }
    void MediaTrackConstraintSet::SetEntry(const std::string & key, double value) {
        SetEntry(key, Format("%f", value));
    }
    
    MediaTrackConstraints::MediaTrackConstraints()
    {
        adapter_ = std::make_shared<MediaTrackConstraintAdapter>(*this);
    }
    
    MediaTrackConstraints::~MediaTrackConstraints()
    {
        
    }
    
    webrtc::MediaConstraintsInterface & MediaTrackConstraints::inner_constraints() {
        auto & ret = *adapter_;
        printf("[MediaTrackConstraints::inner_constraints] %p\n", &ret);
        return ret;
    }
    
    const webrtc::MediaConstraintsInterface & MediaTrackConstraints::inner_constraints() const {
        auto & ret = *adapter_;
        printf("[MediaTrackConstraints::inner_constraints] %p\n", &ret);
        return ret;
    }
    
    MediaTrackConstraintSet & MediaTrackConstraints::mandatory() {
        return mandatory_;
    }
    
    const MediaTrackConstraintSet & MediaTrackConstraints::mandatory() const {
        return mandatory_;
    }
    
    MediaTrackConstraintSet & MediaTrackConstraints::optional() {
        return optional_;
    }
    
    const MediaTrackConstraintSet & MediaTrackConstraints::optional() const {
        return optional_;
    }
}
}