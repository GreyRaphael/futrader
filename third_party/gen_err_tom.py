import re


if __name__ == "__main__":
    pat = re.compile(r'value="(\d+)" prompt="(.+?)"/>')
    with open("./ctp/Linux/error.xml") as fin:
        txt = fin.read()

    result = pat.findall(txt)
    print(f"record length={len(result)}")

    with open("../errors.toml", "w") as fout:
        for k, v in result:
            fout.write(f'{k} = "{v}"\n')
