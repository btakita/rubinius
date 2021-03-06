$: << 'lib' # so we can run in MRI

require File.dirname(__FILE__) + "/spec_helper"

$-w = true

##
# Two dimensions:
# + one vs many
# + splat on end vs not
#
# Other edge cases:
# + no block args
# + empty block args
# + paren args (for like, each_with_index on a hash)
# + nested paren args
# + unnamed splat
# + trailing comma "arg"

describe Compiler do
  it "compiles 'ary.each do; end'" do
    ruby = <<-EOC
        ary.each do; end
      EOC

    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             nil)

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      # do nothing
    end
  end

  it "compiles 'ary.each do ||; end'" do
    ruby = <<-EOC
        ary.each do ||; end
      EOC

    # TODO: ruby_parser SHOULD be using nil for empty body
    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             0)

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      # do nothing
    end
  end

  it "compiles 'ary.each do |a|; end'" do
    ruby = <<-EOC
        ary.each do |a|; end
      EOC

    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             s(:lasgn, :a))

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      d.cast_for_single_block_arg
      d.set_local_depth 0, 0
    end
  end

  it "compiles 'ary.each do |a, |; end'" do
    ruby = <<-EOC
        ary.each do |a, |; end
      EOC

    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             s(:masgn, s(:array, s(:lasgn, :a))))

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      d.cast_for_multi_block_arg
      d.cast_array
      d.lvar_at 0
    end
  end

  it "compiles 'ary.each do |*a|; end'" do
    ruby = <<-EOC
        ary.each do |*a|; end
      EOC

    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             s(:masgn, s(:array, s(:splat, s(:lasgn, :a)))))

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      d.cast_array
      d.cast_array
      d.set_local_depth 0, 0
    end
  end

  it "compiles 'ary.each do |a, b, c|; end'" do
    ruby = <<-EOC
        ary.each do |a, b, c|; end
      EOC

    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             s(:masgn,
               s(:array,
                 s(:lasgn, :a),
                 s(:lasgn, :b),
                 s(:lasgn, :c))))

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      d.cast_for_multi_block_arg
      d.cast_array
      3.times do |slot|
        d.lvar_at slot
      end
    end
  end

  it "compiles 'ary.each do |a, b, *c|; end'" do
    ruby = <<-EOC
        ary.each do |a, b, *c|; end
      EOC

    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             s(:masgn,
               s(:array,
                 s(:lasgn, :a),
                 s(:lasgn, :b),
                 s(:splat, s(:lasgn, :c)))))

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      d.cast_for_multi_block_arg
      d.cast_array
      d.lvar_at 0
      d.lvar_at 1
      d.cast_array
      d.set_local_depth 0, 2
    end
  end

  it "compiles 'ary.each do |(a, b), c|; end'" do
    ruby = <<-EOC
        ary.each do |(a, b), c|; end
      EOC

    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             s(:masgn,
               s(:array,
                 s(:masgn,
                   s(:array,
                     s(:lasgn, :a),
                     s(:lasgn, :b))),
                 s(:lasgn, :c))))

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      d.cast_for_multi_block_arg
      d.cast_array

      # Pull the first element out and use it like a tuple.
      d.shift_array
      d.cast_array
      d.lvar_at 0
      d.lvar_at 1
      d.pop
      d.push :true

      d.pop

      d.lvar_at 2
    end
  end

  it "compiles 'ary.each do |(a, b), *c|; end'" do
    ruby = <<-EOC
        ary.each do |(a, b), *c|; end
      EOC

    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             s(:masgn,
               s(:array,
                 s(:masgn,
                   s(:array,
                     s(:lasgn, :a),
                     s(:lasgn, :b))),
                 s(:splat, s(:lasgn, :c)))))

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      d.cast_for_multi_block_arg
      d.cast_array
      d.shift_array
      d.cast_array

      d.lvar_at 0
      d.lvar_at 1

      d.pop
      d.push :true
      d.pop

      d.cast_array
      d.set_local_depth 0, 2

    end
  end

  it "compiles 'ary.each do |(a, (b, c)), d|; end'" do
    ruby = <<-EOC
        ary.each do |(a, (b, c)), d|; end
      EOC

    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             s(:masgn,
               s(:array,
                 s(:masgn,
                   s(:array,
                     s(:lasgn, :a),
                     s(:masgn,
                       s(:array,
                         s(:lasgn, :b),
                         s(:lasgn, :c))))),
                 s(:lasgn, :d))))

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      d.cast_for_multi_block_arg
      d.cast_array
      d.shift_array
      d.cast_array

      d.lvar_at 0

      d.shift_array
      d.cast_array

      d.lvar_at 1
      d.lvar_at 2

      2.times do
        d.pop
        d.push :true
        d.pop
      end

      d.lvar_at 3
    end
  end

  it "compiles 'ary.each do |*|; end'" do
    ruby = <<-EOC
        ary.each do |*|; end
      EOC

    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             s(:masgn, s(:array, s(:splat))))

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      d.cast_for_multi_block_arg
      d.cast_array
    end
  end

  it "compiles 'ary.each do |a, *|; end'" do
    ruby = <<-EOC
        ary.each do |a, *|; end
      EOC

    sexp = s(:iter,
             s(:call, s(:call, nil, :ary, s(:arglist)), :each, s(:arglist)),
             s(:masgn, s(:array, s(:lasgn, :a), s(:splat))))

    sexp.should == parse(ruby)

    gen_iter sexp do |d|
      d.cast_for_multi_block_arg
      d.cast_array
      d.lvar_at 0
    end
  end

  # TODO: move these to spec/ruby/1.8/language once `rake spec:update` works
  it "runs 'ary.each do; end'" do
    i = 0
    %w(a b c d e f g).each { i += 1 }
    i.should == 7
  end

  it "runs 'ary.each do ||; end'" do
    i = 0
    %w(a b c d e f g).each { || i += 1 } # TODO: check || should warn or not
    i.should == 7
  end

  it "runs 'ary.each do |a|; end'" do
    data = [[1, 2, 3], [4, 5], 6]

    data.dup.map { |a| a }.should == data.dup
  end

  it "runs 'ary.each do |a, |; end'" do
    data = [[1, 2, 3], [4, 5, 6], 7]

    data.dup.map { |a, | a }.should == [1, 4, 7]
  end

  it "runs 'ary.each do |*a|; end'" do
    data = [[1, 2, 3], [4, 5], 6]
    expected = [[[1, 2, 3]], [[4, 5]], [6]]

    data.dup.map { |*a| a }.should == expected
  end

  it "runs 'ary.each do |a, b, c|; end'" do
    data = [[1, 2, 3], [4, 5], 6]
    expected = [[1, 2, 3], [4, 5, nil], [6, nil, nil]]

    data.dup.map { |a, b, c| [a, b, c] }.should == expected
  end

  it "runs 'ary.each do |a, b, *c|; end'" do
    data = [[1, 2, 3], [4, 5], 6]
    expected = [[1, 2, [3]], [4, 5, []], [6, nil, []]]

    data.dup.map { |a, b, *c| [a, b, c] }.should == expected
  end

  it "runs 'ary.each do |(a, b), c|; end'" do
    data = [[[1, 2], 3], [[4], 5], 6]
    expected = [[1, 2, 3], [4, nil, 5], [6, nil, nil]]

    data.dup.map { |(a, b), c| [a, b, c] }.should == expected
  end

  it "runs 'ary.each do |(a, b), *c|; end'" do
    data = [[[1, 2], 3], [[4], 5], 6]
    expected = [[1, 2, [3]], [4, nil, [5]], [6, nil, []]]

    data.dup.map { |(a, b), *c| [a, b, c] }.should == expected
  end

  it "runs 'ary.each do |(a, (b, c)), d|; end'" do
    data     = [[[1, [2, 3]], 4], [[5, 6], 7], [[8], 9], [[10]]]
    expected = [[1, 2, 3, 4], [5, 6, nil, 7],
                [8, nil, nil, 9], [10, nil, nil, nil]]

    data.dup.map { |(a, (b, c)), d| [a, b, c, d] }.should == expected
  end

  it "runs 'ary.each do |*|; end'" do
    i = 0
    %w(a b c d e f g).each { |*| i += 1 }
    i.should == 7
  end

  it "runs 'ary.each do |a, *|; end'" do
    data = [[1, 2, 3], [4, 5], 6]
    expected = [1, 4, 6]

    data.dup.map { |a, *| a }.should == expected
  end
end
