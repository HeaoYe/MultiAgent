import matplotlib.pyplot as plt


def main():
    agents = {}
    t = 0

    while (True):
        try:
            line = input()
        except EOFError:
            break
    
        if line.startswith('frame'):
            dt = float(line.split('dt: ')[1].split(',')[0])
            t += dt
        elif line.startswith('['):
            name, infos = line[1:].split(']')
            for var in 'xvu':
                try:
                    n = float(infos.split(f'{var}: ')[1])
                except ValueError:
                    n = float(infos.split(f'{var}: ')[1].split(',')[0])
                label = f'{name} - {var}'
                try:
                    agents[label][0].append(t)
                    agents[label][1].append(n)
                except KeyError:
                    agents[label] = ([t], [n])
    
    plt.figure()
    i = 1
    for var in 'xvu':
        for name in agents:
            if name.endswith(var):
                plt.subplot(2, 1 if var == 'x' else 2, i)
                plt.title(var)
                plt.plot(*agents[name], label=name.split(' ')[0])
        i += 1
        if var == 'x':
            plt.legend()
            i += 1
    plt.show()


if __name__ == '__main__':
    main()
