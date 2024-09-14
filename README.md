Doing case study on 2d circular motion simulation in a vaccuum space.
Requirements:
- SDL2
- SDL2_ttf

To compile and link with the targeted libraries:
`gcc -o output main.c -lm -LSDL2 -LSDL2_ttf`

To run the compiled elf:
`./output -gf -cor -balls`
=> -gf: gravitation force
=> -cor: coefficient of restitution (bounciness)
=> -balls: the amount of balls to be simulated

