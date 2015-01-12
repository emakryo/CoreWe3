library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.util.all;

entity core_main is
  generic (
    wtime : std_logic_vector(15 downto 0) := x"023D");
  port (
    clk   : in    std_logic;
    RS_TX : out   std_logic;
    RS_RX : in    std_logic;
    ZD    : inout std_logic_vector(31 downto 0);
    ZA    : out   std_logic_vector(19 downto 0);
    XWA   : out   std_logic);
end core_main;

architecture arch_core_main of core_main is

  component init_code_rom
    port (
      inst : out std_logic_vector(31 downto 0);
      pc   : in  unsigned(ADDR_WIDTH-1 downto 0));
  end component;

  component bootload_code_rom is
    generic (
      wtime : std_logic_vector(15 downto 0) := wtime);
    port (
      clk   : in  std_logic;
      RS_RX : in  std_logic;
      ready : out std_logic;
      addr  : in  unsigned(ADDR_WIDTH-1 downto 0);
      instr : out std_logic_vector(31 downto 0));
  end component;

  component memory_io
    generic (
      wtime : std_logic_vector(15 downto 0) := wtime);
    port (
      clk   : in    std_logic;
      RS_RX : in    std_logic;
      RS_TX : out   std_logic;
      ZD    : inout std_logic_vector(31 downto 0);
      ZA    : out   std_logic_vector(19 downto 0);
      XWA   : out   std_logic;
      memi  : in    mem_in_t;
      memo  : out   mem_out_t);
  end component;

  component control is
    port(
      clk   : in  std_logic;
      memo  : in  mem_out_t;
      memi  : out mem_in_t;
      ready : in  std_logic;
      inst  : in  std_logic_vector(31 downto 0);
      pc    : out unsigned(ADDR_WIDTH-1 downto 0));
  end component;

  signal inst  : std_logic_vector(31 downto 0);
  signal pc    : unsigned(ADDR_WIDTH-1 downto 0);
  signal memi  : mem_in_t;
  signal memo  : mem_out_t;
  signal ready : std_logic;
  signal RS_RX_init : std_logic;
  signal RS_RX_exec : std_logic;

begin

  --rom : init_code_rom port map (
  --  inst => inst,
  --  pc   => pc);
  --ready <= '1';


  RS_RX_init <= RS_RX when ready = '0' else
                '1';

  RS_RX_exec <= RS_RX when ready = '1' else
                '1';

  rom : bootload_code_rom port map (
    clk   => clk,
    RS_RX => RS_RX_init,
    ready => ready,
    addr  => pc,
    instr => inst);

  mem : memory_io port map (
    clk   => clk,
    RS_RX => RS_RX_exec,
    RS_TX => RS_TX,
    ZD    => ZD,
    ZA    => ZA,
    XWA   => XWA,
    memi  => memi,
    memo  => memo);

  contoller : control port map (
    clk   => clk,
    memo  => memo,
    memi  => memi,
    ready => ready,
    inst  => inst,
    pc    => pc);

end arch_core_main;
