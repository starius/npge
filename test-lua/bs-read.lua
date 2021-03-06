do
    local bs = BlockSet.new()
    bs:read(">a\nATGC\n>a_1_1 block=b1 norow\n")
    local blocks = bs:iter_blocks()
    local b1 = blocks()
    local f1 = b1:front()
    assert(f1:length() == 1)
    local s = f1:seq()
    local bs2 = BlockSet.new()
    bs2:add_sequence(s)
    local seqs = bs:seqs()
    local s1 = seqs[1]
    local bs3 = BlockSet.new()
    bs3:add_sequence(s1)
end

