//
//  ContentView.swift
//  EditorTest
//
//  Created by Michael Sjoeberg on 2024-01-14.
//

import SwiftUI
import SwiftyMonaco

struct ContentView: View {
    @State var text: String = "test"
    
    var body: some View {
        SwiftyMonaco(text: $text)
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
