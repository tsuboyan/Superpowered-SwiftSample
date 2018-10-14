//
//  SPPlayer.swift
//  Superpowered-SwiftSample
//
//  Created by Atsushi Otsubo on 2017/08/11.
//  Copyright © 2017-2018 Rirex. All rights reserved.
//

import UIKit

class SPPlayer: NSObject {
    
    var superpowered: Superpowered!
    
    override init() {
        superpowered = Superpowered()
        superpowered.toggle()
    }
    
    func setSong(url: URL) {
        superpowered.setSong(url)
    }
    
    func play() {
        superpowered.play()
    }
    
    func pause() {
        superpowered.pause()
    }
    
    func setSeek(percent: Float) {
        superpowered.seek(to: percent)
    }
    
    func jumpToHead() {
        setSeek(percent: 0.0)
    }
    
    func setTempo(tempo: Float) {
        superpowered.setTempo(tempo)
    }
    
    func setPitch(pitch: Int) {
        superpowered.setPitch(Int32(pitch))
    }
    
    func setLoop(loop: Bool) {
        superpowered.setLoop(loop)
    }
    
}
