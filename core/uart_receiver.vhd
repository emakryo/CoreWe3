library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity uart_receiver is
  generic (
    wtime : std_logic_vector(15 downto 0) := x"1ADB");
  port (
    clk  : in  std_logic;
    rx   : in  std_logic;
    complete : out std_logic;
    data : out std_logic_vector(7 downto 0));
end uart_receiver;

architecture arch_uart_receiver of uart_receiver is
  signal countdown : std_logic_vector(15 downto 0) := (others=>'0');
  signal receivebuf : std_logic_vector(7 downto 0) := (others=>'1');
  signal state : std_logic_vector(3 downto 0) := "1001";
begin  -- structure
  statemachine2: process(clk)
    begin
      if rising_edge(clk) then
        case state  is
          when "1001" =>
            if rx='0' then
              receivebuf<=(others => '1');
              state<=state-1;
              countdown<=wtime+("00"&wtime(15 downto 2));
            end if;
            complete <= '0';
          when "0000" =>
            if countdown<="000"&(countdown(15 downto 3)) then
              data<=receivebuf;
              complete<='1';
              state<="1001";
            else
              countdown<=countdown-1;
            end if;
          when others =>
            if countdown=0 then
              receivebuf<=rx&receivebuf(7 downto 1);
              countdown<=wtime;
              state<=state-1;
            else
              countdown<=countdown-1;
            end if;
        end case;
      end if;
    end process;
end arch_uart_receiver;