int option_key_rm_start(char *argv2_p)
{
    char rm_command[9 + strlen(argv2_p)];
    sprintf(rm_command, "rm save/%s", argv2_p);
    system(rm_command);

    return 0;
}