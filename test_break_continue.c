int i = 0;
while (i < 10) {
    i = i + 1;
    if (i == 5) {
        break;
    }
    print(i);
}

int j = 0;
while (j < 5) {
    j = j + 1;
    if (j == 3) {
        continue;
    }
    print(j);
}
