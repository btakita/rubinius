shared :file_writable do |cmd, klass|
  describe "#{klass}.#{cmd}" do
    before :each do
      @file = '/tmp/i_exist'
    end

    after :each do
      File.delete(@file) if File.exists?(@file)
    end

    it "returns true if named file is writable by the effective user id of the process, otherwise false" do
      klass.send(cmd, 'fake_file').should == false
      klass.send(cmd, '/etc/passwd').should == false
      File.open(@file,'w') { klass.send(cmd, @file).should == true }
    end
  end
end