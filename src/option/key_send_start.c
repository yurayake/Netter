int option_key_send_start(char *argv2_p)
{
    char filename[6 + strlen(argv2_p)];
    sprintf(filename, "save/%s", argv2_p);
    
    FILE *fp1 = fopen(filename, "r");
    if (fp1 == NULL)
    {
        perror("fopen");
    }

    // 1行目の文字数を調べる
    int count = 0;
    while (fgetc(fp1) != '\n')
    {
        count++;
    }

    fclose(fp1);

    // 1行目の文字列を格納する配列を定義
    char line_1[count + 1];

    FILE *fp2 = fopen(filename, "r");
    if (fp2 == NULL)
    {
        perror("fopen");
    }

    fscanf(fp2, "%s", line_1);

    //一行目の構文解析
    if (strcmp("using_layer2_layer3", line_1) == 0)
    {
        layer2_layer3_asm(filename);
    }
    else if (strcmp("using_layer1_layer2_layer3", line_1) == 0)
    {
        layer1_layer2_layer3_asm(filename);
    }
    else if (strcmp("using_layer1", line_1) == 0)
    {
        layer1_asm(filename);
    }
    return 0;
}