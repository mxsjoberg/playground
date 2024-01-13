// x86 codegen

struct codegen {
    var code: [String] = []

    mutating func generate(for node: Any) {

        // operators
        if let node = node as? [Any] {
            if let op = node.first as? String {
                let node = node.last as! [Any]
                switch op {
                case "+":
                    generate(for: node[0])
                    code.append("pushq %rax")
                    generate(for: node[1])
                    code.append("popq %rcx")
                    code.append("addq %rcx, %rax")
                case "*":
                    generate(for: node[0])
                    code.append("pushq %rax")
                    generate(for: node[1])
                    code.append("popq %rcx")
                    code.append("imulq %rcx, %rax")
                default:
                    break
                }
            }
        }
        // integer constant
        else if let value = node as? Int {
            code.append("movq $\(value), %rax")
        }
    
    }

    func get_asm() -> String {
        return code.joined(separator: "\n")
    }
}

// usage
var generator = codegen()
let ast: [Any] = ["+", [["*", [2, 2]], 2]]

generator.generate(for: ast)

let asm = generator.get_asm()

print(asm)