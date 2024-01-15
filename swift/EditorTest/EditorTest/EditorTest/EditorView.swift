//
//  EditorView.swift
//  EditorTest
//
//  Created by Michael Sjoeberg on 2024-01-14.
//

import SwiftUI
import SwiftyMonaco

struct EditorView: View {
    @State private var code = "Hello, SwiftyMonaco!"

    var body: some View {
        VStack {
            SwiftyMonaco(
                text: $code,
                language: .swift,
                theme: .vsDark,
            )
            .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity)
        }
    }
}

@main
struct EditorTestApp: App {
    var body: some Scene {
        WindowGroup {
            EditorView()
        }
    }
}
