//
//  ViewController.swift
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/10.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

import UIKit

class ViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
    }

    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        let vc = DebugMenuViewController(nibName: "DebugMenuViewController", bundle: nil)
        self.presentViewController(vc, animated: true, completion: nil)
    }

}

