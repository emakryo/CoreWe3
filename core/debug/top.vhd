library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

library UNISIM;
use UNISIM.VComponents.all;

entity top is
  port (
    MCLK1 : in std_logic;
    RS_RX : in std_logic;
    RS_TX : out std_logic);
end top;

architecture arch of top is
  component IO_buffer_loopback is
  --component transmit_buffer_test is
  --component IO_buffer_test is
    port (
      clk : in std_logic;
      RS_RX : in std_logic;
      RS_TX : out std_logic);
  end component;

  signal clk : std_logic;
  signal iclk : std_logic;
begin
  ib : IBUFG port map (
    i => MCLK1,
    o => iclk);

  bg : BUFG port map (
    i => iclk,
    o => clk);

  main : IO_buffer_loopback port map
  --main : transmit_buffer_test port map
  --main : IO_buffer_test port map
    (clk => clk,
     RS_RX => RS_RX,
     RS_TX => RS_TX);

end arch;
    
