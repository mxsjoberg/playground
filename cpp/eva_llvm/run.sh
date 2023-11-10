# compile
clang++ eva-llvm.cpp -o eva-llvm $(llvm-config --cxxflags --ldflags --system-libs --libs core)

# run compiler
./eva-llvm

# run
lli ./out.ll