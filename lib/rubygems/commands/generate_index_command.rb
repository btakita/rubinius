require 'rubygems/command'
require 'rubygems/indexer'

class Gem::Commands::GenerateIndexCommand < Gem::Command

  def initialize
    super 'generate_index',
          'Generates the index files for a gem server directory',
          :directory => '.'

    add_option '-d', '--directory=DIRNAME',
               'repository base dir containing gems subdir' do |dir, options|
      options[:directory] = File.expand_path dir
    end
  end

  def defaults_str # :nodoc:
    "--directory ."
  end

  def description # :nodoc:
    <<-EOF
The generate_index command creates a set of indexes for serving gems
statically.  The command expects a 'gems' directory under the path given to
the --directory option.  The given diretory will be the directory you serve
as the gem repository.

For `gem generate_index --directory /path/to/repo`, expose /path/to/repo via
your HTTP server configuration (not /path/to/repo/gems).

When done, it will generate a set of files like this:

  gems/*.gem                                   # .gem files you want to
                                               # index

  latest_specs.<version>.gz                    # latest specs index
  specs.<version>.gz                           # specs index
  quick/Marshal.<version>/<gemname>.gemspec.rz # Marshal quick index file

  # these files support legacy RubyGems
  quick/index
  quick/index.rz                               # quick index manifest
  quick/<gemname>.gemspec.rz                   # legacy YAML quick index
                                               # file
  Marshal.<version>
  Marshal.<version>.Z                          # Marshal full index
  yaml
  yaml.Z                                       # legacy YAML full index

The .Z and .rz extension files are compressed with the inflate algorithm.
The Marshal version number comes from ruby's Marshal::MAJOR_VERSION and
Marshal::MINOR_VERSION constants.  It is used to ensure compatibility.
The yaml indexes exist for legacy RubyGems clients and fallback in case of
Marshal version changes.
    EOF
  end

  def execute
    if not File.exist?(options[:directory]) or
       not File.directory?(options[:directory]) then
      alert_error "unknown directory name #{directory}."
      terminate_interaction 1
    else
      indexer = Gem::Indexer.new options[:directory]
      indexer.generate_index
    end
  end

end

