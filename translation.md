```python
for i in range(100):
    print(i)
print(i)

for j in range(100):
    if j > i:
        break
    print(j)
else:
    print(j)
```

```cpp
int main() {
    int i;
    for (i = 0; i < 100; i++) {
        std::cout << i << std::endl;
    }
    std::cout << i << std::endl;

    int j;
    for (j = 0; j < 100; j++) {
        if (j > i) {
            break;
        }
        std::cout << j << std::endl;
    }
    if (j == 100) {
        std::cout << j << std::endl;
    }

    return 0;
}
```

```rust
fn main() {
    for i in 0..100 {
        println!("{}", i);
    }
    println!("{}", i);

    for j in 0..100 {
        if j > i {
            break;
        }
        println!("{}", j);
    }
    println!("{}", j);
}
```
