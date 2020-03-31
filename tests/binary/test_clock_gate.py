import delegator
import pytest

def test_inline_example():
    cur_dir = 'tests/binary'
    in_file = f"{cur_dir}/src/mantle_reg.json"
    res_file = f"{cur_dir}/build/mantle_reg_inline_single_instances.json"
    gold_file = f"{cur_dir}/gold/mantle_reg_inline_single_instances.json"
    res = delegator.run(f"bin/coreir -i {in_file} -l commonlib -p \"rungenerators; inline_single_instances\" -o {res_file}")
    assert not res.return_code, res.out + res.err
    res = delegator.run(f"diff {res_file} {gold_file}")
    assert not res.return_code, res.out + res.err


@pytest.mark.parametrize("suffix", ["json", "v"])
def test_clock_gate(suffix):
    cur_dir = 'tests/binary'
    in_file = f"{cur_dir}/src/mantle_reg.json"
    res_file = f"{cur_dir}/build/mantle_reg_ce.{suffix}"
    gold_file = f"{cur_dir}/gold/mantle_reg_ce.{suffix}"
    res = delegator.run(f"bin/coreir -i {in_file} -l commonlib -p \"rungenerators; inline_single_instances; clock_gate;\" -o {res_file}")
    assert not res.return_code, res.out + res.err
    res = delegator.run(f"diff {res_file} {gold_file}")
    assert not res.return_code, res.out + res.err

    #Test verilator linting if suffix is v
    if suffix == "v":
        res = delegator.run(f"verilator --lint-only {res_file}")
        assert not res.return_code, res.out + res.err
