//
//  CodeEditorDemoDocument.swift
//  EditorTest
//
//  Created by Michael Sjoeberg on 2024-01-14.
//

import SwiftUI
import UniformTypeIdentifiers

extension UTType {
  static var swiftSource: UTType {
    UTType(importedAs: "public.swift-source")
  }
}

struct CodeEditorDemoDocument: FileDocument {
  var text: String

  init(text: String = "") {
    self.text = text
  }

  static var readableContentTypes: [UTType] { [.swiftSource] }

  init(configuration: ReadConfiguration) throws {
    guard let data = configuration.file.regularFileContents,
          let string = String(data: data, encoding: .utf8)
    else {
      throw CocoaError(.fileReadCorruptFile)
    }
    text = string
  }

  func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
    let data = text.data(using: .utf8)!
    return .init(regularFileWithContents: data)
  }
}
