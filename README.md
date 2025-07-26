# Duckdb Extension for reading PLINK2 pgen files

[PLINK2](https://www.cog-genomics.org/plink/2.0/) pgen files are more like a container that can contain genomic data in various forms (modes). This only reads a subset. Reguardless of type of pgen, max number of variants is 2,147,483,645 (0x7ffffffd) anf max number of samples is 2,147,483,645 (0x7ffffffe).

**NOT FULLY IMPLEMENTED**

## Install

For developing see [duckdb-extension](./README-duckdb-extension.md) docs.

TBD

## Running the extension

With extension loaded and in the root direcotry of the extension 

```sql
CREATE TEMP TABLE psam AS SELECT row_number() OVER () AS sample_number, * FROM
    read_csv('test-data/example.psam',
        delim = '\t',
        header = true,
        columns = {
            'IID': 'VARCHAR',
            'SEX': 'VARCHAR'
        });
CREATE TEMP TABLE pvar AS SELECT row_number() OVER () AS sample_number, * FROM
    read_csv('test-data/example.pvar',
        delim = '\t',
        header = true,
        nullstr = ".",
        columns = {
            'CHROM': 'VARCHAR',
            'POS': 'VARCHAR',
            'ID': 'VARCHAR',
            'REF': 'VARCHAR',
            'ALT': 'VARCHAR',
            'QUAL': 'VARCHAR',
            'FILTERD': 'VARCHAR'
        });
CREATE TEMP TABLE pgen AS SELECT * FROM read_pgen('test-data/example.pgen');
```

