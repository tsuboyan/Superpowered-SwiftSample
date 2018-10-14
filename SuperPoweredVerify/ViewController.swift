//
//  ViewController.swift
//  SuperPoweredVerify
//
//  Created by AtsushiOtsubo on 2017/07/13.
//  Copyright © 2017 Rirex. All rights reserved.
//

import UIKit
import MediaPlayer

class ViewController: UIViewController, MPMediaPickerControllerDelegate {
    
    var spPlayer: SPPlayer!
    
    @IBOutlet weak var tempoSlider: UISlider!
    @IBOutlet weak var pitchSlider: UISlider!
    @IBOutlet weak var seekSlider: UISlider!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        spPlayer = SPPlayer()
        NotificationCenter.default.addObserver(self, selector: #selector(self.spPlayerEnded), name: NSNotification.Name(rawValue: "spPlayerEnded"), object: nil)
        setAVAudioSession()
    }
    
    @IBAction func selectSongButton(_ sender: Any) {
        let picker = MPMediaPickerController()
        picker.delegate = self
        picker.allowsPickingMultipleItems = false
        present(picker, animated: true, completion: nil)
    }
    
    // When you piked song
    func mediaPicker(_ mediaPicker: MPMediaPickerController, didPickMediaItems mediaItemCollection: MPMediaItemCollection) {
        defer {
            dismiss(animated: true, completion: nil)
        }
        let items = mediaItemCollection.items
        if items.isEmpty {
            return
        }
        let item = items[0]
        if let url: URL = item.assetURL {
            spPlayer.pause()
            spPlayer.setSong(url: url)
            spPlayer.play()
        }
    }
    
    func mediaPickerDidCancel(_ mediaPicker: MPMediaPickerController) {
        dismiss(animated: true, completion: nil)
    }
    
    @IBAction func playButtonTapped(_ sender: Any) {
        spPlayer.play()
    }
    
    @IBAction func stopButtonTapped(_ sender: Any) {
        spPlayer.pause()
    }
    
    @IBAction func tempoSliderChanged(_ sender: Any) {
        spPlayer.setTempo(tempo: tempoSlider.value)
    }
    
    @IBAction func pitchSliderChanged(_ sender: Any) {
        spPlayer.setPitch(pitch: Int(pitchSlider!.value))
    }
    
    @IBAction func seekSliderChanged(_ sender: Any) {
        spPlayer.setSeek(percent: seekSlider.value)
    }
    
    func spPlayerEnded(notification: Notification?) {
        print("PlayerEnded")
    }
    
    // Support background play
    func setAVAudioSession() {
        let session = AVAudioSession.sharedInstance()
        do {
            try session.setCategory(AVAudioSessionCategoryPlayback)
        } catch  {
            fatalError("Category setting failed")
        }
        do {
            try session.setActive(true)
        } catch {
            fatalError("Session enabled failed")
        }
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
    }
}
